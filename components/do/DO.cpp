// The author disclaims copyright to this source code.
#include "DO.h"

static const char *TAG = "DO";

DO::DO() {
}

DO::~DO() {
}

void DO::setup(gpio_num_t pin, bool on, const char* topic) {
	ESP_LOGD(TAG, "setup, pin:%d, on:%d", pin, on);

	if (pin < GPIO_NUM_0 || pin >= GPIO_NUM_MAX) {
		ESP_LOGE(TAG, "setup requires GPIO pin number (FATAL)");
		return;
	}
	if (topic == 0) {
		ESP_LOGE(TAG, "setup requires topic (FATAL)");
		return;
	}

    // big queue not useful
    QueueHandle_t queue = xQueueCreate(10, sizeof(pubsub_message_t));
    if (queue == 0) {
        ESP_LOGE(TAG, "setup, failed to create queue (FATAL)");
        return;
    }
    pubsub_add_subscription(queue, topic);

	this->pin = pin;
	this->on = on;
	this->queue = queue;

	gpio_pad_select_gpio(pin);
	gpio_set_direction(pin, GPIO_MODE_OUTPUT);
	BaseType_t ret = xTaskCreate(&task, TAG, 2048, this,
			(tskIDLE_PRIORITY + 1), NULL);
	if (ret != pdPASS) {
		ESP_LOGE(TAG, "setup, failed to create task (FATAL)");
	}
}

void DO::run() {
	pubsub_message_t message;
	while (true) {
		if (xQueueReceive(queue, &message, portMAX_DELAY)) {
			ESP_LOGD(TAG, "DO pin:%d=%s", pin, message.boolean_val ? "on" : "off");
			// on value | pin
			//  0     0 |   1
            //  0     1 |   0
			//  1     0 |   0
			//  1     1 |   1
			gpio_set_level(pin, message.boolean_val == on);
		}
	};
}

/**
 * Link C static world to C++ world
 */
void DO::task(void *pvParameter) {
	if (pvParameter == 0) {
		ESP_LOGE(TAG, "task, invalid parameter (FATAL)");
	} else {
		DO *pInstance = (DO*) pvParameter;
		pInstance->run();
	}
}
