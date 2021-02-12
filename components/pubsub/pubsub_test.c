// The author disclaims copyright to this source code.

#include "pubsub.h"
#include "pubsub_test.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <string.h>

static char* pubsub_test_int = "pubsub.test.int";

bool pubsub_test() {
	bool succes = false;
	// create topic
	pubsub_topic_t* topic;
	topic = pubsub_register_topic(pubsub_test_int);
	// publish value
	pubsub_publish_int(topic, 10);
	// subscribe topic
	QueueHandle_t queue = xQueueCreate(10, sizeof(pubsub_message_t));
	pubsub_add_subscription(queue, pubsub_test_int);
	// publish value
	pubsub_publish_int(topic, 11);
	// check received value
	pubsub_message_t message;
	BaseType_t result = xQueueReceive(queue, &message, 2);
	if (result == pdTRUE) {
		if (message.int_val == 11)
		succes = true;
	}

	// cleanup
	pubsub_remove_subscription(queue, pubsub_test_int);
	pubsub_unregister_topic(pubsub_test_int);
	vQueueDelete(queue);

	return succes;

}
