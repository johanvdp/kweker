// The author disclaims copyright to this source code.

#include "string.h"

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "NVS.h"

static const char *TAG = "NVS";

NVS::NVS()
{
}

NVS::~NVS()
{
}

bool NVS::init_topics(const char *topic_list[], const size_t number_of_topics)
{
    if (topic_list == 0 || number_of_topics == 0) {
        return false;
    }

    // maintain a fixed last message for each topic
    // no need for dynamic allocation bookkeeping later
    messages = (pubsub_message_t**) (malloc(sizeof(pubsub_message_t*) * number_of_topics));
    changed = (bool*) (malloc(sizeof(bool) * number_of_topics));
    for (int topic_index = 0; topic_index < number_of_topics; topic_index++) {
        const char *topic_name = topic_list[topic_index];
        pubsub_type_t topic_type = pubsub_get_type(topic_name);
        ESP_LOGI(TAG, "init_topics, topic:%s, type:%d", topic_name, topic_type);
        pubsub_message_t *message = (pubsub_message_t*) (malloc(sizeof(pubsub_message_t)));
        message->topic = topic_name;
        message->type = topic_type;
        messages[topic_index] = message;
        changed[topic_index] = false;
    }
    number_of_messages = number_of_topics;
    return true;
}

bool NVS::init_queue()
{
    // big queue not useful
    QueueHandle_t queue = xQueueCreate(number_of_messages * 2, sizeof(pubsub_message_t));
    if (queue == 0) {
        return false;
    }
    this->queue = queue;
    return true;
}

bool NVS::init_nvs()
{
    bool success = false;
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "init_nvs, nvs_flash_init (%s)", esp_err_to_name(err));
        err = nvs_flash_erase();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "init_nvs, nvs_flash_erase (%s)", esp_err_to_name(err));
        } else {
            err = nvs_flash_init();
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "init_nvs, retry nvs_flash_init (%s)", esp_err_to_name(err));
            } else {
                success = true;
            }
        }
    } else {
        success = true;
    }

    if (success) {
        err = nvs_open(ns, NVS_READWRITE, &handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "init_nvs, nvs_open (%s)", esp_err_to_name(err));
            success = false;
        }
    }
    return success;
}

bool NVS::subscribe_topics()
{
    for (int topic_index = 0; topic_index < number_of_messages; topic_index++) {
        const pubsub_message_t *message = messages[topic_index];
        ESP_LOGI(TAG, "subscribe_topics, topic:%s", message->topic);
        pubsub_add_subscription(this->queue, message->topic, false);
    }
    return true;
}

void NVS::setup(const char *ns, const char *topic_list[], const size_t number_of_topics, const uint32_t hold_off_period_ms)
{
    ESP_LOGI(TAG, "setup, namespace:%s, topics:%p[%d]", ns, topic_list, number_of_topics);

    if (ns == 0) {
        ESP_LOGE(TAG, "setup, requires namespace (FATAL)");
        return;
    }
    this->ns = ns;

    if (hold_off_period_ms == 0) {
        ESP_LOGE(TAG, "setup, requires hold off period ms (FATAL)");
        return;
    }
    this->hold_off_period_ticks = hold_off_period_ms / portTICK_PERIOD_MS;

    if (!init_topics(topic_list, number_of_topics)) {
        ESP_LOGE(TAG, "setup, requires topics (FATAL)");
        return;
    }
    if (!init_queue()) {
        ESP_LOGE(TAG, "setup, failed to create queue (FATAL)");
        return;
    }
    if (!init_nvs()) {
        ESP_LOGE(TAG, "setup, failed to initialize NVS (FATAL)");
        return;
    }
    if (!subscribe_topics()) {
        return;
    }

    read_nvs();

    BaseType_t ret = xTaskCreate(&task, TAG, 2048, this, (tskIDLE_PRIORITY + 1), NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "setup, failed to create task (FATAL)");
    }
}

void NVS::read_nvs()
{
    for (int topic_index = 0; topic_index < number_of_messages; topic_index++) {
        pubsub_message_t *message = messages[topic_index];
        esp_err_t err = ESP_ERR_NVS_BASE;
        if (message->type == PUBSUB_TYPE_INT) {
            int64_t value = 0;
            size_t size = sizeof(int64_t);
            err = nvs_get_blob(handle, message->topic, &value, &size);
            ESP_LOGI(TAG, "read_nvs, key:%s, value:%lld", message->topic, value);
            message->int_val = value;
            pubsub_publish_int(message->topic, value);
        } else if (message->type == PUBSUB_TYPE_DOUBLE) {
            double value = 0.0;
            size_t size = sizeof(double);
            err = nvs_get_blob(handle, message->topic, &value, &size);
            ESP_LOGI(TAG, "read_nvs, key:%s, value:%lf", message->topic, value);
            message->double_val = value;
            pubsub_publish_double(message->topic, value);
        } else if (message->type == PUBSUB_TYPE_BOOLEAN) {
            bool value = false;
            size_t size = sizeof(bool);
            err = nvs_get_blob(handle, message->topic, &value, &size);
            ESP_LOGI(TAG, "read_nvs, key:%s, value:%s", message->topic, value ? "true" : "false");
            message->boolean_val = value;
            pubsub_publish_bool(message->topic, value);
        } else {
            ESP_LOGE(TAG, "read_nvs, unsupported message type:%d", message->type);
        }
        if (err == ESP_OK) {
            // read successful, no need to store value
            changed[topic_index] = false;
        } else {
            ESP_LOGW(TAG, "read_nvs, failed (%s)", esp_err_to_name(err));
            // read failed, need to store value
            changed[topic_index] = true;
        }
    }
}

void NVS::run()
{
    pubsub_message_t message;
    while (true) {
        if (xQueueReceive(queue, &message, hold_off_period_ticks)) {
            ESP_LOGI(TAG, "run, update topic:%s", message.topic);
            bool found = false;
            for (int i = 0; i < number_of_messages; i++) {
                if (strcmp(messages[i]->topic, message.topic) == 0) {
                    found = true;
                    pubsub_message_t *last = messages[i];
                    last->type = message.type;
                    if (last->type == PUBSUB_TYPE_INT) {
                        // avoid ringing, mark only changes
                        if (last->int_val != message.int_val) {
                            changed[i] = true;
                            last->int_val = message.int_val;
                        }
                    } else if (last->type == PUBSUB_TYPE_DOUBLE) {
                        // avoid ringing, mark only changes
                        if (last->double_val != message.double_val) {
                            changed[i] = true;
                            last->double_val = message.double_val;
                        }
                    } else if (last->type == PUBSUB_TYPE_BOOLEAN) {
                        // avoid ringing, mark only changes
                        if (last->boolean_val != message.boolean_val) {
                            changed[i] = true;
                            last->boolean_val = message.boolean_val;
                        }
                    } else {
                        ESP_LOGE(TAG, "run, unsupported message type:%d", last->type);
                    }
                    // only expect one
                    break;
                }
            }
            if (!found) {
                ESP_LOGE(TAG, "run, unknown topic:%s", message.topic);
            }
        } else {
            // message queue timeout, time to perform write actions
            write_nvs();
        }
    }
}

void NVS::write_nvs()
{
    if (is_change_detected()) {
        ESP_LOGI(TAG, "write_nvs, change detected");
        for (int i = 0; i < number_of_messages; i++) {
            if (changed[i]) {
                pubsub_message_t *message = messages[i];
                esp_err_t err = ESP_ERR_NVS_BASE;
                if (message->type == PUBSUB_TYPE_INT) {
                    ESP_LOGI(TAG, "write_nvs, key:%s, value:%lld", message->topic, message->int_val);
                    err = nvs_set_blob(handle, message->topic, &message->int_val, sizeof(int64_t));
                } else if (message->type == PUBSUB_TYPE_DOUBLE) {
                    ESP_LOGI(TAG, "write_nvs, key:%s, value:%lf", message->topic, message->double_val);
                    err = nvs_set_blob(handle, message->topic, &message->double_val, sizeof(double));
                } else if (message->type == PUBSUB_TYPE_BOOLEAN) {
                    ESP_LOGI(TAG, "write_nvs, key:%s, value:%s", message->topic, message->boolean_val ? "true" : "false");
                    err = nvs_set_blob(handle, message->topic, &message->int_val, sizeof(bool));
                } else {
                    ESP_LOGE(TAG, "write_nvs, unsupported message type:%d", message->type);
                }
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "write_nvs, failed (%s)", esp_err_to_name(err));
                }
                // mark as unchanged
                changed[i] = false;
            }
        }
        nvs_commit(handle);
    }
}

bool NVS::is_change_detected()
{
    for (int i = 0; i < number_of_messages; i++) {
        if (changed[i]) {
            return true;
        }
    }
    return false;
}

void NVS::task(void *pvParameter)
{
    if (pvParameter == 0) {
        ESP_LOGE(TAG, "task, invalid parameter (FATAL)");
    } else {
        NVS *pInstance = (NVS*) pvParameter;
        pInstance->run();
    }
}
