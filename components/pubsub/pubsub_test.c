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

bool pubsub_test() {
	ESP_LOGI(tag, "pubsub_test");

	int succes = 0;
	// create topic
	pubsub_topic_t *topic_int;
	topic_int = pubsub_register_topic(TOPIC_PUBSUB_TEST_INT);
	pubsub_topic_t *topic_bool;
	topic_bool = pubsub_register_topic(TOPIC_PUBSUB_TEST_BOOL);
	pubsub_topic_t *topic_double;
	topic_double = pubsub_register_topic(TOPIC_PUBSUB_TEST_DOUBLE);
	// publish value
	pubsub_publish_int(topic_int, 10);
	pubsub_publish_bool(topic_bool, 1);
	pubsub_publish_double(topic_double, 0.10);
	// subscribe topic
	QueueHandle_t queueMixed = xQueueCreate(5, sizeof(pubsub_message_t));
	pubsub_add_subscription(queueMixed, TOPIC_PUBSUB_TEST_INT);
	pubsub_add_subscription(queueMixed, TOPIC_PUBSUB_TEST_BOOL);
	pubsub_add_subscription(queueMixed, TOPIC_PUBSUB_TEST_DOUBLE);
	QueueHandle_t queueInt = xQueueCreate(5, sizeof(pubsub_message_t));
	pubsub_add_subscription(queueInt, TOPIC_PUBSUB_TEST_INT);
	// publish value
	pubsub_publish_int(topic_int, 11);
	pubsub_publish_bool(topic_bool, 0);
	pubsub_publish_double(topic_double, 0.11);
	// check received value
	pubsub_message_t message;
	BaseType_t result = xQueueReceive(queueMixed, &message, 2);
	if (result == pdTRUE) {

		if (message.int_val == 11) {
			ESP_LOGI(tag, "ok 11");
			succes++;
		}
	}
	result = xQueueReceive(queueMixed, &message, 2);
	if (result == pdTRUE) {
		if (message.boolean_val == 0) {
			ESP_LOGI(tag, "ok false");
			succes++;
		}
	}
	result = xQueueReceive(queueMixed, &message, 2);
	if (result == pdTRUE) {
		if (message.double_val > 0.10 && message.double_val < 0.12) {
			ESP_LOGI(tag, "ok 0.11");
			succes++;
		}
	}
	result = xQueueReceive(queueInt, &message, 2);
	if (result == pdTRUE) {
		if (message.int_val == 11) {
			ESP_LOGI(tag, "ok 11");
			succes++;
		}
	}
	result = xQueueReceive(queueInt, &message, 1);
	// expect queue empty
	if (result != pdTRUE) {
		ESP_LOGI(tag, "ok empty");
		succes++;
	}

	// cleanup
	pubsub_remove_subscription(queueMixed, TOPIC_PUBSUB_TEST_BOOL);
	pubsub_remove_subscription(queueMixed, TOPIC_PUBSUB_TEST_DOUBLE);
	pubsub_remove_subscription(queueMixed, TOPIC_PUBSUB_TEST_INT);
	pubsub_remove_subscription(queueInt, TOPIC_PUBSUB_TEST_INT);
	pubsub_unregister_topic(TOPIC_PUBSUB_TEST_BOOL);
	pubsub_unregister_topic(TOPIC_PUBSUB_TEST_DOUBLE);
	pubsub_unregister_topic(TOPIC_PUBSUB_TEST_INT);
	vQueueDelete(queueMixed);
	vQueueDelete(queueInt);

	return succes == 5;

}
