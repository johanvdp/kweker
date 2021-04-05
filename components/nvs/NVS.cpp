// The author disclaims copyright to this source code.

#include "string.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "NVS.h"

static const char *TAG = "NVS";

NVS::NVS()
{
}

NVS::~NVS()
{
}

void NVS::setup(const char *ns, const char *topic_list[], const size_t number_of_topics, const uint32_t hold_off_period_ms)
{
    ESP_LOGI(TAG, "setup, namespace:%s, topics:%p[%d]", ns, topic_list, number_of_topics);

    if (ns == 0) {
        ESP_LOGE(TAG, "setup requires namespace (FATAL)");
        return;
    }
    this->ns = ns;

    if (topic_list == 0 || number_of_topics == 0) {
        ESP_LOGE(TAG, "setup requires topics (FATAL)");
        return;
    }

    if (hold_off_period_ms == 0) {
        ESP_LOGE(TAG, "setup requires hold off period ms (FATAL)");
        return;
    }
    this->hold_off_period_ticks = hold_off_period_ms / portTICK_PERIOD_MS;

    // big queue not useful
    QueueHandle_t queue = xQueueCreate(10, sizeof(pubsub_message_t));
    if (queue == 0) {
        ESP_LOGE(TAG, "setup, failed to create queue (FATAL)");
        return;
    }
    this->queue = queue;

    // subscribe to topics
    keys = (const char**) malloc(sizeof(const char*) * number_of_topics);
    values = (pubsub_message_t**) malloc(sizeof(pubsub_message_t*) * number_of_topics);
    for (int topic_index = 0; topic_index < number_of_topics; topic_index++) {
        const char *topic = topic_list[topic_index];
        ESP_LOGI(TAG, "setup, subscribe topic:%s", topic);

        // maintain a fixed last message for each topic
        // no need for dynamic allocation bookkeeping later
        *(keys + topic_index) = topic;
        pubsub_message_t *message = (pubsub_message_t*) malloc(sizeof(pubsub_message_t));
        *(values + topic_index) = message;
        message->topic = topic;
        // using type to mark as empty
        message->type = PUBSUB_TYPE_UNKNOWN;

        pubsub_add_subscription(queue, topic, false);
    }
    number_of_keys = number_of_topics;

    BaseType_t ret = xTaskCreate(&task, TAG, 2048, this, (tskIDLE_PRIORITY + 1), NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "setup, failed to create task (FATAL)");
    }
}

void NVS::run()
{
    pubsub_message_t message;
    bool perform_save_action = false;
    while (true) {
        if (xQueueReceive(queue, &message, hold_off_period_ticks)) {
            ESP_LOGI(TAG, "run, update topic:%s", message.topic);
            // find key
            for (int i = 0; i < number_of_keys; i++) {
                if (strcmp(*(keys + i), message.topic) == 0) {
                    // found
                    perform_save_action = true;
                    pubsub_message_t *last = *(values + i);
                    last->type = message.type;
                    if (last->type == PUBSUB_TYPE_INT) {
                        last->int_val = message.int_val;
                    } else if (last->type == PUBSUB_TYPE_DOUBLE) {
                        last->double_val = message.double_val;
                    } else if (last->type == PUBSUB_TYPE_BOOLEAN) {
                        last->boolean_val = message.boolean_val;
                    } else {
                        ESP_LOGE(TAG, "run, unsupported message type:%d", last->type);
                    }
                    // only expect one
                    break;
                }
            }
        } else {
            // queue empty but update was requested
            if (perform_save_action) {
                perform_save_action = false;
                for (int i = 0; i < number_of_keys; i++) {
                    const char *key = *(keys + i);
                    pubsub_message_t *message = *(values + i);
                    if (message->type != PUBSUB_TYPE_UNKNOWN) {
                        // update
                        if (message->type == PUBSUB_TYPE_INT) {
                            ESP_LOGI(TAG, "update key:%s, value:%lld", key, message->int_val);
                        } else if (message->type == PUBSUB_TYPE_DOUBLE) {
                            ESP_LOGI(TAG, "update key:%s, value:%lf", key, message->double_val);
                        } else if (message->type == PUBSUB_TYPE_BOOLEAN) {
                            ESP_LOGI(TAG, "update key:%s, value:%s", key, message->boolean_val ? "true" : "false");
                        } else {
                            ESP_LOGE(TAG, "run, unsupported message type:%d", message->type);
                        }
                        // done, mark as empty
                        message->type = PUBSUB_TYPE_UNKNOWN;
                    }
                }
            }
        }
    };
}

/**
 * Link C static world to C++ world
 */
void NVS::task(void *pvParameter)
{
    if (pvParameter == 0) {
        ESP_LOGE(TAG, "task, invalid parameter (FATAL)");
    } else {
        NVS *pInstance = (NVS*) pvParameter;
        pInstance->run();
    }
}
