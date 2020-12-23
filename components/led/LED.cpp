// The author disclaims copyright to this source code.
#include "LED.h"

// why is this required here?
#include "driver/gpio.h"

static const char *tag = "LED";

LED::LED() {
}

LED::~LED() {
}

void LED::signal(uint8_t count) {
	ESP_LOGD(tag, "signal, count: %d", count);

	if (queue == 0) {
		ESP_LOGE(tag, "signal no queue (FATAL)");
	} else {
		// deliver or drop
		BaseType_t ret = xQueueSend(queue, &count, (TickType_t ) 1);
		if (ret != pdPASS) {
			// might recover
			ESP_LOGE(tag, "signal xQueueSend failed");
		}
	}
}

void LED::setup(gpio_num_t pin, bool on) {
	ESP_LOGD(tag, "setup, pin: %d, on: %d", pin, on);

	if (pin < GPIO_NUM_0 || pin >= GPIO_NUM_MAX) {
		ESP_LOGE(tag, "setup requires GPIO pin number (FATAL)");
		return;
	}
	this->pin = pin;
	this->on = on;

	gpio_pad_select_gpio(pin);
	gpio_set_direction(pin, GPIO_MODE_OUTPUT);
	// big queue not useful
	queue = xQueueCreate(10, sizeof(uint8_t));
	if (queue == 0) {
		ESP_LOGE(tag, "setup, failed to create queue (FATAL)");
	} else {
		BaseType_t ret = xTaskCreate(&task, "setup", 2048, this,
				(tskIDLE_PRIORITY + 2), NULL);
		if (ret != pdPASS) {
			ESP_LOGE(tag, "setup, failed to create task (FATAL)");
		}
	}
}

void LED::run() {
	while (true) {
		if (queue == 0) {
			ESP_LOGE(tag, "run, no queue (FATAL)");
		} else {
			uint8_t count;
			if (xQueueReceive(queue, &count, portMAX_DELAY)) {
				// blink series
				for (int i = 0; i < count; i++) {
					gpio_set_level(pin, on);
					vTaskDelay(20 / portTICK_PERIOD_MS);
					gpio_set_level(pin, !on);
					vTaskDelay(80 / portTICK_PERIOD_MS);
				}
				// separate blink series
				vTaskDelay(500 / portTICK_PERIOD_MS);
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
