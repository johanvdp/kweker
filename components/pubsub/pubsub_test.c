// The author disclaims copyright to this source code.

#include "pubsub.h"
#include "pubsub_test.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <string.h>
#include "esp_log.h"

static const char *tag = "pubsub_test";

static const char *TOPIC_PUBSUB_TEST_INT = "pubsub.test.int";
static const char *TOPIC_PUBSUB_TEST_BOOL = "pubsub.test.bool";
static const char *TOPIC_PUBSUB_TEST_DOUBLE = "pubsub.test.double";

bool pubsub_test()
{
    ESP_LOGI(tag, "pubsub_test");

    bool success = true;

    // create topic
    pubsub_topic_t *topic_int;
    topic_int = pubsub_register_topic(TOPIC_PUBSUB_TEST_INT, PUBSUB_TYPE_INT);
    pubsub_topic_t *topic_bool;
    topic_bool = pubsub_register_topic(TOPIC_PUBSUB_TEST_BOOL,
            PUBSUB_TYPE_BOOLEAN);
    pubsub_topic_t *topic_double;
    topic_double = pubsub_register_topic(TOPIC_PUBSUB_TEST_DOUBLE,
            PUBSUB_TYPE_DOUBLE);

    // check
    if (pubsub_topic_count() != 3) {
        ESP_LOGE(tag, "expect 3 topics");
        success = false;
    }

    // publish value (no subscribers)
    pubsub_publish_int(topic_int, 9);
    pubsub_publish_bool(topic_bool, 1);
    pubsub_publish_double(topic_double, 0.09);

    // examine last value
    int64_t int_value = 0;
    pubsub_last_int(topic_int, &int_value);
    if (int_value != 9) {
        ESP_LOGE(tag, "expect last value 9");
        success = false;
    }

    bool bool_value = false;
    pubsub_last_bool(topic_bool, &bool_value);
    if (bool_value != true) {
        ESP_LOGE(tag, "expect last value true");
        success = false;
    }

    double double_value = 0.0;
    pubsub_last_double(topic_double, &double_value);
    if (double_value < 0.08 || double_value > 0.10) {
        ESP_LOGE(tag, "expect last value 0.09");
        success = false;
    }

    // subscribe topic
    QueueHandle_t queueMixed = xQueueCreate(5, sizeof(pubsub_message_t));
    pubsub_add_subscription(queueMixed, TOPIC_PUBSUB_TEST_INT);
    pubsub_add_subscription(queueMixed, TOPIC_PUBSUB_TEST_BOOL);
    pubsub_add_subscription(queueMixed, TOPIC_PUBSUB_TEST_DOUBLE);
    QueueHandle_t queueInt = xQueueCreate(5, sizeof(pubsub_message_t));
    pubsub_add_subscription(queueInt, TOPIC_PUBSUB_TEST_INT);

    // check
    if (pubsub_subscriber_count(TOPIC_PUBSUB_TEST_BOOL) == 1) {
        ESP_LOGE(tag, "expect 1 subscriber");
        success = false;
    }
    if (pubsub_subscriber_count(TOPIC_PUBSUB_TEST_DOUBLE) == 1) {
        ESP_LOGE(tag, "expect 1 subscriber");
        success = false;
    }
    if (pubsub_subscriber_count(TOPIC_PUBSUB_TEST_INT) == 2) {
        ESP_LOGE(tag, "expect 2 subscribers");
        success = false;
    }

    // publish value
    pubsub_publish_int(topic_int, 11);
    pubsub_publish_bool(topic_bool, true);
    pubsub_publish_double(topic_double, 0.11);

    // check received value
    pubsub_message_t message;
    if (xQueueReceive(queueMixed, &message, 1) != pdTRUE
            || message.int_val != 11) {
        ESP_LOGE(tag, "expect 11");
        success = false;
    }
    if (xQueueReceive(queueMixed, &message, 1) != pdTRUE
            || message.boolean_val == 0) {
        ESP_LOGE(tag, "expect true");
        success = false;
    }
    if (xQueueReceive(queueMixed, &message, 1) != pdTRUE
            || message.double_val < 0.10 || message.double_val > 0.12) {
        ESP_LOGE(tag, "expect 0.11");
        success = false;
    }
    if (xQueueReceive(queueInt, &message, 1) != pdTRUE
            || message.int_val != 11) {
        ESP_LOGE(tag, "expect 11");
        success = false;
    }
    if (xQueueReceive(queueInt, &message, 1) == pdTRUE) {
        ESP_LOGE(tag, "expect queue empty");
        success = false;
    }

    // examine last value
    int_value = 0;
    pubsub_last_int(topic_int, &int_value);
    if (int_value != 11) {
        ESP_LOGE(tag, "expect last value 11");
        success = false;
    }

    bool_value = false;
    pubsub_last_bool(topic_bool, &bool_value);
    if (bool_value != true) {
        ESP_LOGE(tag, "expect last value true");
        success = false;
    }

    double_value = 0.0;
    pubsub_last_double(topic_double, &double_value);
    if (double_value < 0.10 || double_value > 0.12) {
        ESP_LOGE(tag, "expect last value 0.11");
        success = false;
    }

    // remove subscriptions
    pubsub_remove_subscription(queueMixed, TOPIC_PUBSUB_TEST_BOOL);
    pubsub_remove_subscription(queueMixed, TOPIC_PUBSUB_TEST_DOUBLE);
    pubsub_remove_subscription(queueMixed, TOPIC_PUBSUB_TEST_INT);
    pubsub_remove_subscription(queueInt, TOPIC_PUBSUB_TEST_INT);

    // check
    if (pubsub_topic_count() != 3) {
        ESP_LOGE(tag, "expect 3 topics");
        success = false;
    }
    if (pubsub_subscriber_count(TOPIC_PUBSUB_TEST_BOOL) != 0) {
        ESP_LOGE(tag, "expect 0 subscriber");
        success = false;
    }
    if (pubsub_subscriber_count(TOPIC_PUBSUB_TEST_DOUBLE) != 0) {
        ESP_LOGE(tag, "expect 0 subscriber");
        success = false;
    }
    if (pubsub_subscriber_count(TOPIC_PUBSUB_TEST_INT) != 0) {
        ESP_LOGE(tag, "expect 0 subscribers");
        success = false;
    }

    // unregister topic
    pubsub_unregister_topic(TOPIC_PUBSUB_TEST_BOOL);
    pubsub_unregister_topic(TOPIC_PUBSUB_TEST_DOUBLE);
    pubsub_unregister_topic(TOPIC_PUBSUB_TEST_INT);

    // check
    if (pubsub_topic_count() != 0) {
        ESP_LOGE(tag, "expect 0 topics");
        success = false;
    }
    if (pubsub_subscriber_count(TOPIC_PUBSUB_TEST_BOOL) != 0) {
        ESP_LOGE(tag, "expect 0 subscriber");
        success = false;
    }
    if (pubsub_subscriber_count(TOPIC_PUBSUB_TEST_DOUBLE) != 0) {
        ESP_LOGE(tag, "expect 0 subscriber");
        success = false;
    }
    if (pubsub_subscriber_count(TOPIC_PUBSUB_TEST_INT) != 0) {
        ESP_LOGE(tag, "expect 0 subscribers");
        success = false;
    }

    vQueueDelete(queueMixed);
    vQueueDelete(queueInt);

    return success;
}
