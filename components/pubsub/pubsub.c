// The author disclaims copyright to this source code.

#include "pubsub.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <sys/queue.h>
#include <string.h>
#include "esp_log.h"

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
    ESP_LOGV(tag, "pubsub_find_topic_detail topic:%s", topic_name);
    pubsub_topic_detail_t *candidate;
    LIST_FOREACH(candidate, &pubsub_topic_subscribers, pointers)
    {
        if (strcmp(candidate->topic, topic_name) == 0) {
            // found
            ESP_LOGV(tag, "pubsub_find_topic_detail found:%p", candidate);
            return candidate;
        }
    }
    ESP_LOGV(tag, "pubsub_find_topic_detail found:none");
    return NULL;
}

static pubsub_subscriber_t* pubsub_create_subscriber(
        QueueHandle_t subscriber_queue)
{
    ESP_LOGV(tag, "pubsub_create_subscriber queue:%p", subscriber_queue);

    pubsub_subscriber_t *subscriber = (pubsub_subscriber_t*) malloc(
            sizeof(pubsub_subscriber_t));
    subscriber->queue = subscriber_queue;
    return subscriber;
}

/**
 * Add topic name to list.
 * @return the topic.
 */
static pubsub_topic_detail_t* pubsub_add_topic_detail(const char *topic_name,
        pubsub_type_t type)
{
    ESP_LOGV(tag, "pubsub_add_topic_detail topic:%s", topic_name);
    pubsub_topic_detail_t *topic_detail = (pubsub_topic_detail_t*) malloc(
            sizeof(pubsub_topic_detail_t));
    topic_detail->topic = strdup(topic_name);
    topic_detail->type = type;
    if (type == PUBSUB_TYPE_INT) {
        topic_detail->int_val = 0;
    } else if (type == PUBSUB_TYPE_DOUBLE) {
        topic_detail->double_val = 0;
    } else if (type == PUBSUB_TYPE_BOOLEAN) {
        topic_detail->boolean_val = false;
    } else {
        ESP_LOGE(tag, "pubsub_add_topic_detail invalid type:%d", type);
    }
    LIST_INSERT_HEAD(&pubsub_topic_subscribers, topic_detail, pointers);
    LIST_INIT(&(topic_detail->subscribers));
    ESP_LOGV(tag, "pubsub_add_topic_detail subscribers:%p", topic_detail);
    return topic_detail;
}

/**
 * Add subscription to topic name.
 */
void pubsub_add_subscription(QueueHandle_t subscriber_queue,
        const char *topic_name)
{
    ESP_LOGI(tag, "pubsub_add_subscription topic:%s, queue:%p", topic_name,
            subscriber_queue);

    // find existing topic by name
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail == NULL) {
        ESP_LOGE(tag, "pubsub_add_subscription unknown topic:%s", topic_name);
        return;
    }
    // add subscriber to topic
    pubsub_subscriber_t *subscriber = pubsub_create_subscriber(
            subscriber_queue);
    LIST_INSERT_HEAD(&(topic_detail->subscribers), subscriber, pointers);
}

/**
 * Remove subscription to topic
 */
void pubsub_remove_subscription(QueueHandle_t subscriber_queue,
        const char *topic_name)
{
    ESP_LOGI(tag, "pubsub_remove_subscription topic:%s, queue:%p", topic_name,
            subscriber_queue);
    // find existing topic by name
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail != NULL) {
        // remove subscriber from topic
        pubsub_subscriber_t *candidate;
        LIST_FOREACH(candidate, &(topic_detail->subscribers), pointers)
        {
            if (candidate->queue == subscriber_queue) {
                ESP_LOGD(tag,
                        "pubsub_remove_subscription topic:%s, queue:%p found",
                        topic_name, subscriber_queue);
                LIST_REMOVE(candidate, pointers);
                free(candidate);
                break;
            }
        }
    }
}

pubsub_topic_t pubsub_register_topic(const char *topic_name,
        const pubsub_type_t type)
{
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail == NULL) {
        topic_detail = pubsub_add_topic_detail(topic_name, type);
        ESP_LOGI(tag, "pubsub_register_topic new topic:%s, topic:%p",
                topic_name, topic_detail);
    } else {

        if (topic_detail->type != type) {
            ESP_LOGE(tag,
                    "pubsub_register_topic existing topic:%s, topic:%p, type:%d, mismatch new type:%d",
                    topic_name, topic_detail, topic_detail->type, type);
        } else {
            ESP_LOGI(tag, "pubsub_register_topic existing topic:%s, topic:%p",
                    topic_name, topic_detail);
        }
    }
    return (pubsub_topic_t) topic_detail;
}

void pubsub_unregister_topic(const char *topic_name)
{
    ESP_LOGI(tag, "pubsub_unregister_topic topic:%s", topic_name);
    pubsub_topic_detail_t *topic_detail = pubsub_find_topic_detail(topic_name);
    if (topic_detail != NULL) {
        ESP_LOGD(tag, "pubsub_unregister_topic topic:%p", topic_detail);
        pubsub_subscriber_t *subscriber;
        pubsub_subscriber_t *subscriber_safe;
        LIST_FOREACH_SAFE(subscriber, &(topic_detail->subscribers), pointers, subscriber_safe)
        {
            ESP_LOGD(tag, "pubsub_unregister_topic queue:%p",
                    subscriber->queue);
            LIST_REMOVE(subscriber, pointers);
            free(subscriber);
        }
        LIST_REMOVE(topic_detail, pointers);
    }
}

void pubsub_publish(pubsub_topic_t topic, pubsub_message_t *message)
{
    pubsub_topic_detail_t *topic_detail = (pubsub_topic_detail_t*) topic;
    if (topic_detail->type != message->type) {
        ESP_LOGE(tag,
                "pubsub_publish type mismatch topic:%s, type:%d, message type:%d",
                topic_detail->topic, topic_detail->type, message->type);
        return;
    }
    ESP_LOGD(tag, "pubsub_publish topic:%p", topic);
    // store last value
    if (topic_detail->type == PUBSUB_TYPE_INT) {
        topic_detail->int_val = message->int_val;
    } else if (topic_detail->type == PUBSUB_TYPE_BOOLEAN) {
        topic_detail->boolean_val = message->boolean_val;
    } else if (topic_detail->type == PUBSUB_TYPE_DOUBLE) {
        topic_detail->double_val = message->double_val;
    } else {
        ESP_LOGE(tag, "pubsub_publish unknown type topic:%s",
                topic_detail->topic);
        return;
    }
    // inform all subscribers
    pubsub_subscriber_t *subscriber;
    LIST_FOREACH(subscriber, &(topic_detail->subscribers), pointers)
    {
        QueueHandle_t queue = subscriber->queue;
        ESP_LOGD(tag, "pubsub_publish queue:%p", queue);
        BaseType_t result = xQueueSendToBack(queue, message, 0);
        if (result != pdTRUE) {
            // todo handle errors
            ESP_LOGE(tag, "publish send error");
        } else {
            ESP_LOGV(tag, "publish send ok");
        }
    }
}

void pubsub_publish_bool(pubsub_topic_t topic, bool value)
{
    const char *str_value = value ? "true" : "false";
    ESP_LOGD(tag, "pubsub_publish_bool topic:%p, value:%s", topic, str_value);
    if (topic == NULL) {
        ESP_LOGE(tag, "pubsub_publish_bool topic required");
        return;
    }
    pubsub_topic_detail_t *topic_detail = (pubsub_topic_detail_t*) topic;
    if (topic_detail->type != PUBSUB_TYPE_BOOLEAN) {
        ESP_LOGE(tag, "pubsub_publish_bool type mismatch topic:%s",
                topic_detail->topic);
        return;
    }
    ESP_LOGI(tag, "pubsub_publish_bool topic:%s, value:%s", topic_detail->topic,
            str_value);
    pubsub_message_t message;
    message.topic = topic_detail->topic;
    message.type = PUBSUB_TYPE_BOOLEAN;
    message.boolean_val = value;
    pubsub_publish(topic, &message);
}

void pubsub_publish_int(pubsub_topic_t topic, int64_t value)
{
    ESP_LOGD(tag, "pubsub_publish_int topic:%p, value:%lld", topic, value);
    if (topic == NULL) {
        ESP_LOGE(tag, "pubsub_publish_int topic required");
        return;
    }
    pubsub_topic_detail_t *topic_detail = (pubsub_topic_detail_t*) topic;
    if (topic_detail->type != PUBSUB_TYPE_INT) {
        ESP_LOGE(tag, "pubsub_publish_int type mismatch topic:%s",
                topic_detail->topic);
        return;
    }
    ESP_LOGI(tag, "pubsub_publish_int topic:%s, value:%lld",
            topic_detail->topic, value);
    pubsub_message_t message;
    message.topic = topic_detail->topic;
    message.type = PUBSUB_TYPE_INT;
    message.int_val = value;
    pubsub_publish(topic, &message);
}

void pubsub_publish_double(pubsub_topic_t topic, double value)
{
    ESP_LOGD(tag, "pubsub_publish_double topic:%p, value:%lf", topic, value);
    if (topic == NULL) {
        ESP_LOGE(tag, "pubsub_publish_double topic required");
        return;
    }
    pubsub_topic_detail_t *topic_detail = (pubsub_topic_detail_t*) topic;
    if (topic_detail->type != PUBSUB_TYPE_DOUBLE) {
        ESP_LOGE(tag, "pubsub_publish_double type mismatch topic:%s",
                topic_detail->topic);
        return;
    }
    ESP_LOGI(tag, "pubsub_publish_double topic:%s, value:%lf",
            topic_detail->topic, value);
    pubsub_message_t message;
    message.topic = topic_detail->topic;
    message.type = PUBSUB_TYPE_DOUBLE;
    message.double_val = value;
    pubsub_publish(topic, &message);
}

uint16_t pubsub_topic_count()
{
    uint16_t count = 0;
    pubsub_topic_detail_t *topic_detail;
    LIST_FOREACH(topic_detail, &pubsub_topic_subscribers, pointers)
    {
        ESP_LOGD(tag, "pubsub_topic_count topic:%s", topic_detail->topic);
        count++;
    }
    ESP_LOGI(tag, "pubsub_topic_count count:%d", count);
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
            ESP_LOGD(tag, "pubsub_subscriber_count queue:%p",
                    subscriber->queue);
            count++;
        }
    }
    ESP_LOGI(tag, "pubsub_subscriber_count topic:%s, count:%d", topic_name,
            count);
    return 0;
}

/**
 * get last published value for topic
 * @return true if successful, false on failure.
 */
bool pubsub_last_bool(pubsub_topic_t topic, bool *value)
{
    pubsub_topic_detail_t *topic_detail = (pubsub_topic_detail_t*) topic;
    if (topic_detail == NULL) {
        ESP_LOGE(tag, "pubsub_last_bool missing topic");
        return false;
    }
    if (topic_detail->type != PUBSUB_TYPE_BOOLEAN) {
        ESP_LOGE(tag, "pubsub_last_bool topic:%s, incorrect type:%d",
                topic_detail->topic, topic_detail->type);
        return false;
    }
    *value = topic_detail->boolean_val;
    return true;
}

/**
 * get last published value for topic
 * @return true if successful, false on failure.
 */
bool pubsub_last_int(pubsub_topic_t topic, int64_t *value)
{
    pubsub_topic_detail_t *topic_detail = (pubsub_topic_detail_t*) topic;
    if (topic_detail == NULL) {
        ESP_LOGE(tag, "pubsub_last_int missing topic");
        return false;
    }
    if (topic_detail->type != PUBSUB_TYPE_INT) {
        ESP_LOGE(tag, "pubsub_last_int topic:%s, incorrect type:%d",
                topic_detail->topic, topic_detail->type);
        return false;
    }
    *value = topic_detail->int_val;
    return true;
}

/**
 * get last published value for topic
 * @return true if successful, false on failure.
 */
bool pubsub_last_double(pubsub_topic_t topic, double *value)
{
    pubsub_topic_detail_t *topic_detail = (pubsub_topic_detail_t*) topic;
    if (topic_detail == NULL) {
        ESP_LOGE(tag, "pubsub_last_double missing topic");
        return false;
    }
    if (topic_detail->type != PUBSUB_TYPE_DOUBLE) {
        ESP_LOGE(tag, "pubsub_last_double topic:%s, incorrect type:%d",
                topic_detail->topic, topic_detail->type);
        return false;
    }
    *value = topic_detail->double_val;
    return true;
}
