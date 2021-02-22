// The author disclaims copyright to this source code.
#include "LED.h"

// why is this required here?
#include "driver/gpio.h"
#include "esp_log.h"

static const char *tag = "LED";

LED::LED() {
}

LED::~LED() {
}

void LED::setup(gpio_num_t pin, bool on, QueueHandle_t queue) {
	ESP_LOGD(tag, "setup, pin: %d, on: %d", pin, on);

	if (pin < GPIO_NUM_0 || pin >= GPIO_NUM_MAX) {
		ESP_LOGE(tag, "setup requires GPIO pin number (FATAL)");
		return;
	}
	if (queue == 0) {
		ESP_LOGE(tag, "signal requires queue (FATAL)");
		return;
	}
	this->pin = pin;
	this->on = on;
	this->queue = queue;

	gpio_pad_select_gpio(pin);
	gpio_set_direction(pin, GPIO_MODE_OUTPUT);
	BaseType_t ret = xTaskCreate(&task, "setup", 2048, this,
			(tskIDLE_PRIORITY + 2), NULL);
	if (ret != pdPASS) {
		ESP_LOGE(tag, "setup, failed to create task (FATAL)");
	}
}

void LED::run() {
	pubsub_message_t message;
	while (true) {
		if (xQueueReceive(queue, &message, portMAX_DELAY)) {
			ESP_LOGI(tag, "xQueueReceive");
			// blink series
			for (int i = 0; i < message.int_val; i++) {
				gpio_set_level(pin, on);
				vTaskDelay(20 / portTICK_PERIOD_MS);
				gpio_set_level(pin, !on);
				vTaskDelay(80 / portTICK_PERIOD_MS);
			}
		}
	};
}

/**
 * Link C static world to C++ world
 */
void LED::task(void *pvParameter) {
	if (pvParameter == 0) {
		ESP_LOGE(tag, "task, invalid parameter (FATAL)");
	} else {
		LED *pInstance = (LED*) pvParameter;
		pInstance->run();
	}
}
