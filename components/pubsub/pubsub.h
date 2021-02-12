// The author disclaims copyright to this source code.

#ifndef _PUBSUB_H_
#define _PUBSUB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum {
	INT, DOUBLE, BOOLEAN
} pubsub_type_t;

typedef struct {
	char *topic;
	pubsub_type_t type;
	union {
		int64_t int_val;
		double double_val;
		bool boolean_val;
	};
} pubsub_message_t;

typedef void* pubsub_topic_t;

extern void pubsub_initialize();

extern void pubsub_add_subscription(QueueHandle_t subscriber_queue, char* topic_name);
extern void pubsub_remove_subscription(QueueHandle_t subscriber_queue, char* topic_name);

extern pubsub_topic_t* pubsub_register_topic(char* topic_name);
extern void pubsub_unregister_topic(char* topic_name);
extern void pubsub_publish(pubsub_topic_t *topic, pubsub_message_t *message);
extern void pubsub_publish_bool(pubsub_topic_t *topic, bool value);
extern void pubsub_publish_int(pubsub_topic_t *topic, int64_t value);
extern void pubsub_publish_double(pubsub_topic_t *topic, double value);

#ifdef __cplusplus
}
#endif

#endif /* _PUBSUB_H_ */
