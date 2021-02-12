// The author disclaims copyright to this source code.
extern "C" {

#include "freertos/FreeRTOS.h"
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

	led.setup(LED_GPIO, true);

	am2301.setup(AM2301_GPIO, am2301ResultQueue);

	AM2301::result_t am2301Result;
	while (1) {

		led.signal(1);
		am2301.measure();
		if (xQueueReceive(am2301ResultQueue, &am2301Result,
				5000 / portTICK_PERIOD_MS)) {
			if (am2301Result.status == AM2301::result_status_t::RESULT_OK) {
				ESP_LOGI(TAG, "T:%.1fC RH:%.1f%%",
						am2301Result.temperature - 273.15,
						am2301Result.humidity);
				led.signal(2);
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
