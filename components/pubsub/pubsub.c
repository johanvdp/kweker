// The author disclaims copyright to this source code.

#include <string.h>
#include <sys/queue.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "pubsub.h"

static const char *tag = "pubsub";

/** Subscriber list element. */
typedef struct pubsub_subscriber_s
{
    QueueHandle_t queue;
    LIST_ENTRY(pubsub_subscriber_s)
    pointers;
} pubsub_subscriber_t;

/** Topic list element. */
typedef struct pubsub_topic_detail_s
{
    char *topic;
    pubsub_type_t type;
    bool always;
    // last known value
    union
    {
        int64_t int_val;
        double double_val;
        bool boolean_val;
    };
    LIST_HEAD(pubsub_subscriber_list, pubsub_subscriber_s)
    subscribers;
    LIST_ENTRY(pubsub_topic_detail_s)
    pointers;
} pubsub_topic_detail_t;

static LIST_HEAD(pubsub_topic_subscribers_list, pubsub_topic_detail_s)
pubsub_topic_subscribers;

/** Initialize once. */
void pubsub_initialize()
{
    LIST_INIT(&pubsub_topic_subscribers);
}

static pubsub_topic_detail_t* pubsub_find_topic_detail(const char *topic_name)
{
    ESP_LOGV(tag, "pubsub_find_topic_detail, topic:%s", topic_name);
    pubsub_topic_detail_t *candidate;
    LIST_FOREACH(candidate, &pubsub_topic_subscribers, pointers)
    {
        if (strcmp(candidate->topic, topic_name) == 0) {
            // found
            ESP_LOGV(tag, "pubsub_find_topic_detail, found:%p", candidate);
            return candidate;
        }
    }
    ESP_LOGV(tag, "pubsub_find_topic_detail, found:none");
    return NULL;
}

static pubsub_subscriber_t* pubsub_create_subscriber(QueueHandle_t subscriber_queue)
{
    ESP_LOGV(tag, "pubsub_create_subscriber, queue:%p", subscriber_queue);

    pubsub_subscriber_t *subscriber = (pubsub_subscriber_t*) malloc(sizeof(pubsub_subscriber_t));
    subscriber->queue = subscriber_queue;
    return subscriber;
}

/**
 * Add topic name to list.
 * @return the topic.
 */
static pubsub_topic_detail_t* pubsub_add_topic_detail(const char *topic_name, pubsub_type_t type, const bool always)
{
    ESP_LOGV(tag, "pubsub_add_topic_detail, topic:%s", topic_name);
    pubsub_topic_detail_t *topic_detail = (pubsub_topic_detail_t*) malloc(sizeof(pubsub_topic_detail_t));
    topic_detail->topic = strdup(topic_name);
    topic_detail->type = type;
    topic_detail->always = always;
    if (type == PUBSUB_TYPE_INT) {
        topic_detail->int_val = 0;
    } else if (type == PUBSUB_TYPE_DOUBLE) {
        topic_detail->double_val = 0;
    } else if (type == PUBSUB_TYPE_BOOLEAN) {
        topic_detail->boolean_val = false;
    } else {
        ESP_LOGE(tag, "pubsub_add_topic_detail, invalid type:%d", type);
    }
    LIST_INSERT_HEAD(&pubsub_topic_subscribers, topic_detail, pointers);
    LIST_INIT(&(topic_detail->subscribers));
    ESP_LOGV(tag, "pubsub_add_topic_detail, subscribers:%p", topic_detail);
    return topic_detail;
}

static void pubsub_publish_one(QueueHandle_t queue, pubsub_message_t *message)
{
    ESP_LOGD(tag, "pubsub_publish_one, queue:%p", queue);
    BaseType_t result = xQueueSendToBack(queue, message, 0);
    if (result != pdTRUE) {
        // todo handle errors
        ESP_LOGE(tag, "pubsub_publish_one, error topic:%s, queue:%p", message->topic, queue);
    } else {
        ESP_LOGV(tag, "pubsub_publish_one, ok topic:%s", message->topic);
    }
}
/**
 * Add subscription to topic name.
 */
void pubsub_add_subscription(QueueHandle_t subscriber_queue, const char *topic_name, bool hot)
{
    ESP_LOGI(tag, "pubsub_add_subscription, topic:%s, queue:%p", topic_name, subscriber_queue);

    // find existing topic by name
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail == NULL) {
        ESP_LOGE(tag, "pubsub_add_subscription, unknown topic:%s", topic_name);
        return;
    }
    // add subscriber to topic
    pubsub_subscriber_t *subscriber = pubsub_create_subscriber(subscriber_queue);
    LIST_INSERT_HEAD(&(topic_detail->subscribers), subscriber, pointers);
    // publish last known value if hot
    if (hot) {
        pubsub_message_t message;
        message.topic = topic_detail->topic;
        message.type = topic_detail->type;
        pubsub_type_t type = topic_detail->type;
        if (type == PUBSUB_TYPE_INT) {
            message.int_val = topic_detail->int_val;
        } else if (type == PUBSUB_TYPE_DOUBLE) {
            message.double_val = topic_detail->double_val;
        } else if (type == PUBSUB_TYPE_BOOLEAN) {
            message.boolean_val = topic_detail->boolean_val;
        } else {
            ESP_LOGE(tag, "pubsub_add_subscription, invalid type:%d", type);
        }
        pubsub_publish_one(subscriber->queue, &message);
    }
}

/**
 * Remove subscription to topic
 */
void pubsub_remove_subscription(QueueHandle_t subscriber_queue, const char *topic_name)
{
    ESP_LOGI(tag, "pubsub_remove_subscription, topic:%s, queue:%p", topic_name, subscriber_queue);
    // find existing topic by name
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail != NULL) {
        // remove subscriber from topic
        pubsub_subscriber_t *candidate;
        LIST_FOREACH(candidate, &(topic_detail->subscribers), pointers)
        {
            if (candidate->queue == subscriber_queue) {
                ESP_LOGD(tag, "pubsub_remove_subscription, topic:%s, queue:%p found", topic_name, subscriber_queue);
                LIST_REMOVE(candidate, pointers);
                free(candidate);
                break;
            }
        }
    }
}

bool pubsub_register_topic(const char *topic_name, const pubsub_type_t type, const bool always)
{
    bool success = false;
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail == NULL) {
        topic_detail = pubsub_add_topic_detail(topic_name, type, always);
        ESP_LOGI(tag, "pubsub_register_topic, new topic:%s, topic:%p", topic_name, topic_detail);

        success = true;
    } else {
        if (topic_detail->type != type) {
            ESP_LOGE(tag, "pubsub_register_topic, existing topic:%s, topic:%p, type:%d, mismatch new type:%d", topic_name,
                    topic_detail, topic_detail->type, type);
        } else if (topic_detail->always != always) {
            ESP_LOGE(tag, "pubsub_register_topic, existing topic:%s, topic:%p, always:%d, mismatch new always:%d", topic_name,
                    topic_detail, topic_detail->always, always);
        } else {
            ESP_LOGI(tag, "pubsub_register_topic, existing topic:%s, topic:%p", topic_name, topic_detail);
        }
    }
    return success;
}

bool pubsub_unregister_topic(const char *topic_name)
{
    bool success = false;
    ESP_LOGI(tag, "pubsub_unregister_topic, topic:%s", topic_name);
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail != NULL) {
        ESP_LOGD(tag, "pubsub_unregister_topic, topic:%p", topic_detail);
        pubsub_subscriber_t *subscriber;
        pubsub_subscriber_t *subscriber_safe;
        LIST_FOREACH_SAFE(subscriber, &(topic_detail->subscribers), pointers, subscriber_safe)
        {
            ESP_LOGD(tag, "pubsub_unregister_topic, queue:%p", subscriber->queue);
            LIST_REMOVE(subscriber, pointers);
            free(subscriber);
        }
        LIST_REMOVE(topic_detail, pointers);
        success = true;
    }
    return success;
}

pubsub_type_t pubsub_get_type(const char *topic_name)
{
    pubsub_type_t topic_type = PUBSUB_TYPE_UNKNOWN;
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail != NULL) {
        topic_type = topic_detail->type;
    }
    return topic_type;
}

void pubsub_publish(const char *topic_name, pubsub_message_t *message)
{
    if (topic_name == NULL) {
        ESP_LOGE(tag, "pubsub_publish, topic required");
        return;
    }
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail == NULL) {
        return;
    }
    if (topic_detail->type != message->type) {
        ESP_LOGE(tag, "pubsub_publish, type mismatch topic:%s, type:%d, message type:%d", topic_detail->topic, topic_detail->type,
                message->type);
        return;
    }
    // check if value changed
    // store last value
    bool value_changed;
    if (topic_detail->type == PUBSUB_TYPE_INT) {
        value_changed = topic_detail->int_val != message->int_val;
        topic_detail->int_val = message->int_val;
        if (topic_detail->always || value_changed) {
            ESP_LOGI(tag, "pubsub_publish, %s=%lld", topic_detail->topic, message->int_val);
        }
    } else if (topic_detail->type == PUBSUB_TYPE_BOOLEAN) {
        value_changed = topic_detail->boolean_val != message->boolean_val;
        topic_detail->boolean_val = message->boolean_val;
        if (topic_detail->always || value_changed) {
            ESP_LOGI(tag, "pubsub_publish, %s=%s", topic_detail->topic, message->boolean_val ? "true" : "false");
        }
    } else if (topic_detail->type == PUBSUB_TYPE_DOUBLE) {
        value_changed = topic_detail->double_val != message->double_val;
        topic_detail->double_val = message->double_val;
        if (topic_detail->always || value_changed) {
            ESP_LOGI(tag, "pubsub_publish, %s=%lf", topic_detail->topic, message->double_val);
        }
    } else {
        value_changed = false;
        ESP_LOGE(tag, "pubsub_publish, unknown type topic:%s", topic_detail->topic);
        return;
    }
    // publish always or if changed
    if (topic_detail->always || value_changed) {
        // inform all subscribers
        pubsub_subscriber_t *subscriber;
        LIST_FOREACH(subscriber, &(topic_detail->subscribers), pointers)
        {
            pubsub_publish_one(subscriber->queue, message);
        }
    }
}

void pubsub_publish_bool(const char *topic_name, bool value)
{
    if (topic_name == NULL) {
        ESP_LOGE(tag, "pubsub_publish_bool, topic required");
        return;
    }
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail == NULL) {
        return;
    }
    const char *str_value = value ? "true" : "false";
    ESP_LOGD(tag, "pubsub_publish_bool, topic:%p, value:%s", topic_name, str_value);
    if (topic_name == NULL) {
        ESP_LOGE(tag, "pubsub_publish_bool, topic required");
        return;
    }
    if (topic_detail->type != PUBSUB_TYPE_BOOLEAN) {
        ESP_LOGE(tag, "pubsub_publish_bool, type mismatch topic:%s", topic_detail->topic);
        return;
    }
    pubsub_message_t message;
    message.topic = topic_detail->topic;
    message.type = PUBSUB_TYPE_BOOLEAN;
    message.boolean_val = value;
    pubsub_publish(topic_name, &message);
}

void pubsub_publish_int(const char *topic_name, int64_t value)
{
    if (topic_name == NULL) {
        ESP_LOGE(tag, "pubsub_publish_int, topic required");
        return;
    }
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail == NULL) {
        return;
    }
    ESP_LOGD(tag, "pubsub_publish_int, topic:%p, value:%lld", topic_name, value);
    if (topic_name == NULL) {
        ESP_LOGE(tag, "pubsub_publish_int, topic required");
        return;
    }
    if (topic_detail->type != PUBSUB_TYPE_INT) {
        ESP_LOGE(tag, "pubsub_publish_int, type mismatch topic:%s", topic_detail->topic);
        return;
    }
    pubsub_message_t message;
    message.topic = topic_detail->topic;
    message.type = PUBSUB_TYPE_INT;
    message.int_val = value;
    pubsub_publish(topic_name, &message);
}

void pubsub_publish_double(const char *topic_name, double value)
{
    if (topic_name == NULL) {
        ESP_LOGE(tag, "pubsub_publish_double, topic required");
        return;
    }
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail == NULL) {
        return;
    }
    ESP_LOGD(tag, "pubsub_publish_double, topic:%p, value:%lf", topic_name, value);
    if (topic_name == NULL) {
        ESP_LOGE(tag, "pubsub_publish_double topic required");
        return;
    }
    if (topic_detail->type != PUBSUB_TYPE_DOUBLE) {
        ESP_LOGE(tag, "pubsub_publish_double, type mismatch topic:%s", topic_detail->topic);
        return;
    }
    pubsub_message_t message;
    message.topic = topic_detail->topic;
    message.type = PUBSUB_TYPE_DOUBLE;
    message.double_val = value;
    pubsub_publish(topic_name, &message);
}

uint16_t pubsub_topic_count()
{
    uint16_t count = 0;
    pubsub_topic_detail_t *topic_detail;
    LIST_FOREACH(topic_detail, &pubsub_topic_subscribers, pointers)
    {
        ESP_LOGD(tag, "pubsub_topic_count, topic:%s", topic_detail->topic);
        count++;
    }
    ESP_LOGI(tag, "pubsub_topic_count, count:%d", count);
    return count;
}

uint16_t pubsub_subscriber_count(const char *topic_name)
{
    uint16_t count = 0;
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    pubsub_subscriber_t *subscriber;
    if (topic_detail != NULL) {
        LIST_FOREACH(subscriber, &(topic_detail->subscribers), pointers)
        {
            ESP_LOGD(tag, "pubsub_subscriber_count, queue:%p", subscriber->queue);
            count++;
        }
    }
    ESP_LOGI(tag, "pubsub_subscriber_count, topic:%s, count:%d", topic_name, count);
    return 0;
}

/**
 * get last published value for topic
 * @return true if successful, false on failure.
 */
bool pubsub_last_bool(const char *topic_name, bool *value)
{
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail == NULL) {
        ESP_LOGE(tag, "pubsub_last_bool, missing topic");
        return false;
    }
    if (topic_detail->type != PUBSUB_TYPE_BOOLEAN) {
        ESP_LOGE(tag, "pubsub_last_bool, topic:%s, incorrect type:%d", topic_detail->topic, topic_detail->type);
        return false;
    }
    *value = topic_detail->boolean_val;
    return true;
}

/**
 * get last published value for topic
 * @return true if successful, false on failure.
 */
bool pubsub_last_int(const char *topic_name, int64_t *value)
{
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail == NULL) {
        ESP_LOGE(tag, "pubsub_last_int, missing topic");
        return false;
    }
    if (topic_detail->type != PUBSUB_TYPE_INT) {
        ESP_LOGE(tag, "pubsub_last_int, topic:%s, incorrect type:%d", topic_detail->topic, topic_detail->type);
        return false;
    }
    *value = topic_detail->int_val;
    return true;
}

/**
 * get last published value for topic
 * @return true if successful, false on failure.
 */
bool pubsub_last_double(const char *topic_name, double *value)
{
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail == NULL) {
        ESP_LOGE(tag, "pubsub_last_double, missing topic");
        return false;
    }
    if (topic_detail->type != PUBSUB_TYPE_DOUBLE) {
        ESP_LOGE(tag, "pubsub_last_double, topic:%s, incorrect type:%d", topic_detail->topic, topic_detail->type);
        return false;
    }
    *value = topic_detail->double_val;
    return true;
}
