// The author disclaims copyright to this source code.
extern "C" {

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "hmi.h"
#include "LED.h"
#include "AM2301.h"
#include "pubsub.h"
#include "pubsub_test.h"

#define TAG "main"

// GPIO configuration see Kconfig.projbuild
#define LED_GPIO (gpio_num_t)CONFIG_LED_GPIO
#define AM2301_GPIO (gpio_num_t)CONFIG_AM2301_GPIO

LED led;
AM2301 am2301;

static const char* TOPIC_ONBOARD_LED = "onboard.led";

void app_main() {
	ESP_LOGI(TAG, "app_main");

	QueueHandle_t am2301ResultQueue = xQueueCreate(2, sizeof(AM2301::result_t));
	if (am2301ResultQueue == 0) {
		ESP_LOGE(TAG, "setup xQueueCreate failed (FATAL)");
		return;
	}

	hmi_initialize();
	pubsub_initialize();
	bool succes = pubsub_test();
	if (succes) {
		ESP_LOGI(TAG, "pubsub_test succes");
	} else {
		ESP_LOGE(TAG, "pubsub_test failed (FATAL)");
		return;
	}

	// big queue not useful
	QueueHandle_t led_queue = xQueueCreate(10, sizeof(pubsub_message_t));
	if (led_queue == 0) {
		ESP_LOGE(TAG, "setup, failed to create queue (FATAL)");
		return;
	}
	ESP_LOGI(TAG, "pubsub_register_topic");
	pubsub_topic_t led_topic = pubsub_register_topic(TOPIC_ONBOARD_LED);
	ESP_LOGI(TAG, "pubsub_add_subscription");
	pubsub_add_subscription(led_queue, TOPIC_ONBOARD_LED);
	ESP_LOGI(TAG, "led.setup");
	led.setup(LED_GPIO, true, led_queue);

	am2301.setup(AM2301_GPIO, am2301ResultQueue);

	AM2301::result_t am2301Result;
	pubsub_message_t led_message;
	while (1) {

		led_message.topic = (char *)TOPIC_ONBOARD_LED;
		led_message.type = INT;
		led_message.int_val = 1;
		ESP_LOGI(TAG, "pubsub_publish 1");
		pubsub_publish(led_topic, &led_message);
		am2301.measure();
		if (xQueueReceive(am2301ResultQueue, &am2301Result,
				5000 / portTICK_PERIOD_MS)) {
			if (am2301Result.status == AM2301::result_status_t::RESULT_OK) {
				ESP_LOGI(TAG, "T:%.1fC RH:%.1f%%",
						am2301Result.temperature - 273.15,
						am2301Result.humidity);
				led_message.int_val = 2;
				ESP_LOGI(TAG, "pubsub_publish 2");
				pubsub_publish(led_topic, &led_message);
			} else if (am2301Result.status
					== AM2301::result_status_t::RESULT_RECOVERABLE) {
				ESP_LOGE(TAG, "app_main xQueueReceive RECOVERABLE");
				// continue
			} else if (am2301Result.status
					== AM2301::result_status_t::RESULT_FATAL) {
				ESP_LOGE(TAG, "app_main xQueueReceive FATAL");
				// give up
				break;
			} else {
				ESP_LOGE(TAG, "app_main status: %d", am2301Result.status);
			}
		}
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}
}
