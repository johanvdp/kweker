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
typedef struct pubsub_subscriber_s {
	QueueHandle_t queue;
	LIST_ENTRY(pubsub_subscriber_s)
	pointers;
} pubsub_subscriber_t;

/** Topic subscribers list element. */
typedef struct pubsub_topic_subscribers_s {
	char *topic;
	LIST_HEAD(pubsub_subscriber_list, pubsub_subscriber_s)
	subscribers;
	LIST_ENTRY(pubsub_topic_subscribers_s)
	pointers;
} pubsub_topic_subscribers_t;

static LIST_HEAD(pubsub_topic_subscribers_list, pubsub_topic_subscribers_s)
pubsub_topic_subscribers;

/** Initialize once. */
void pubsub_initialize() {
	LIST_INIT(&pubsub_topic_subscribers);
}

static pubsub_topic_subscribers_t* pubsub_find_topic_subscribers(
		const char *topic_name) {
	ESP_LOGI(tag, "pubsub_topic_subscribers_t topic:%s", topic_name);
	pubsub_topic_subscribers_t *candidate;
	LIST_FOREACH(candidate, &pubsub_topic_subscribers, pointers)
	{
		if (strcmp(candidate->topic, topic_name) == 0) {
			// found
			ESP_LOGI(tag, "pubsub_topic_subscribers_t found:%p", candidate);
			return candidate;
		}
	}
    ESP_LOGI(tag, "pubsub_topic_subscribers_t none");
	return NULL;
}

static pubsub_subscriber_t* pubsub_create_subscriber(
		QueueHandle_t subscriber_queue) {
	ESP_LOGI(tag, "pubsub_create_subscriber queue:%p", subscriber_queue);

	pubsub_subscriber_t *subscriber = (pubsub_subscriber_t*) malloc(
			sizeof(pubsub_subscriber_t));
	subscriber->queue = subscriber_queue;
	return subscriber;
}

/**
 * Add topic name to list.
 * @return the topic.
 */
static pubsub_topic_subscribers_t* pubsub_add_topic_subscribers(
		const char *topic_name) {
	ESP_LOGI(tag, "pubsub_add_topic_subscribers topic:%s", topic_name);
	pubsub_topic_subscribers_t *topic_subscribers =
			(pubsub_topic_subscribers_t*) malloc(
					sizeof(pubsub_topic_subscribers_t));
	topic_subscribers->topic = strdup(topic_name);
	LIST_INSERT_HEAD(&pubsub_topic_subscribers, topic_subscribers, pointers);
	LIST_INIT(&(topic_subscribers->subscribers));
	ESP_LOGI(tag, "pubsub_add_topic_subscribers subscribers:%p", topic_subscribers);
	return topic_subscribers;
}

/**
 * Add subscription to topic name.
 */
void pubsub_add_subscription(QueueHandle_t subscriber_queue, const char *topic_name) {
	ESP_LOGI(tag, "pubsub_add_subscription topic:%s, queue:%p", topic_name, subscriber_queue);

	// find existing topic by name or add new topic name
	pubsub_topic_subscribers_t *topic_subscribers =
			pubsub_find_topic_subscribers(topic_name);
	if (topic_subscribers == NULL) {
		topic_subscribers = pubsub_add_topic_subscribers(topic_name);
	}
	// add subscriber to topic
	pubsub_subscriber_t *subscriber = pubsub_create_subscriber(
			subscriber_queue);
	LIST_INSERT_HEAD(&(topic_subscribers->subscribers), subscriber, pointers);
}

/**
 * Remove subscription to topic name
 */
void pubsub_remove_subscription(QueueHandle_t subscriber_queue,
		const char *topic_name) {
	ESP_LOGI(tag, "pubsub_remove_subscription queue:%p", subscriber_queue);
	// find existing topic by name
	pubsub_topic_subscribers_t *topic_subscribers =
			pubsub_find_topic_subscribers(topic_name);
	if (topic_subscribers != NULL) {
		// remove subscriber from topic
		pubsub_subscriber_t *candidate;
		LIST_FOREACH(candidate, &(topic_subscribers->subscribers), pointers)
		{
			if (candidate->queue == subscriber_queue) {

				LIST_REMOVE(candidate, pointers);
				free(candidate);
				break;
			}
		}
	}
}

pubsub_topic_t pubsub_register_topic(const char *topic_name) {
	ESP_LOGI(tag, "pubsub_register_topic topic:%s", topic_name);
	pubsub_topic_subscribers_t *topic_subscribers =
			pubsub_find_topic_subscribers(topic_name);
	if (topic_subscribers == NULL) {
		topic_subscribers = pubsub_add_topic_subscribers(topic_name);
	}
	ESP_LOGI(tag, "pubsub_register_topic topic:%p", topic_subscribers);
	return (pubsub_topic_t) topic_subscribers;
}

void pubsub_unregister_topic(const char *topic_name) {
	ESP_LOGI(tag, "pubsub_unregister_topic topic:%s", topic_name);
	pubsub_topic_subscribers_t *topic_subscribers =
			pubsub_find_topic_subscribers(topic_name);
	if (topic_subscribers != NULL) {
		ESP_LOGI(tag, "pubsub_unregister_topic topic:%p", topic_subscribers);
		if (LIST_EMPTY(&(topic_subscribers->subscribers))) {
			LIST_REMOVE(topic_subscribers, pointers);
			free(topic_subscribers->topic);
			free(&(topic_subscribers->subscribers));
		}
	}
}

void pubsub_publish(pubsub_topic_t topic, pubsub_message_t *message) {
	pubsub_topic_subscribers_t *topic_subscribers =
			(pubsub_topic_subscribers_t*) topic;
	ESP_LOGI(tag, "pubsub_publish topic:%p", topic);
	pubsub_subscriber_t *subscriber;
	LIST_FOREACH(subscriber, &(topic_subscribers->subscribers), pointers)
	{
		QueueHandle_t queue = subscriber->queue;
		ESP_LOGI(tag, "pubsub_publish queue:%p", queue);
		BaseType_t result = xQueueSendToBack(queue, message, 0);
		if (result != pdTRUE) {
			// todo handle errors
			ESP_LOGE(tag, "publish send error");
		} else {
			ESP_LOGI(tag, "publish send ok");
		}
	}
}

void pubsub_publish_bool(pubsub_topic_t topic, bool value) {
	pubsub_topic_subscribers_t *topic_subscribers =
			(pubsub_topic_subscribers_t*) topic;
	ESP_LOGI(tag, "pubsub_publish_bool topic:%p, value:%s", topic, value ? "true" : "false");
	pubsub_message_t message;
	message.topic = topic_subscribers->topic;
	message.type = BOOLEAN;
	message.boolean_val = value;
	pubsub_publish((pubsub_topic_t*) topic_subscribers, &message);
}

void pubsub_publish_int(pubsub_topic_t topic, int64_t value) {
	pubsub_topic_subscribers_t *topic_subscribers =
			(pubsub_topic_subscribers_t*) topic;
	ESP_LOGI(tag, "pubsub_publish_int topic:%p, value:%lld", topic, value);
	pubsub_message_t message;
	message.topic = topic_subscribers->topic;
	message.type = INT;
	message.int_val = value;
	pubsub_publish((pubsub_topic_t*) topic_subscribers, &message);
}

void pubsub_publish_double(pubsub_topic_t topic, double value) {
	pubsub_topic_subscribers_t *topic_subscribers =
			(pubsub_topic_subscribers_t*) topic;
	ESP_LOGI(tag, "pubsub_publish_int topic:%p, value:%lf", topic, value);
	pubsub_message_t message;
	message.topic = topic_subscribers->topic;
	message.type = DOUBLE;
	message.double_val = value;
	pubsub_publish((pubsub_topic_t*) topic_subscribers, &message);
}
