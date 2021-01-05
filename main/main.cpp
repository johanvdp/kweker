#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "LED.h"
#include "AM2301.h"

// GPIO configuration see Kconfig.projbuild
#define LED_GPIO (gpio_num_t)CONFIG_LED_GPIO
#define AM2301_GPIO (gpio_num_t)CONFIG_AM2301_GPIO

static const char *tag = "main";

extern "C" {

//int myPrintFunction(const char *format, va_list arg) {
//	return 0;
//}

void app_main(void) {

	//esp_log_set_vprintf(myPrintFunction);

	ESP_LOGD(tag, "app_main");

	QueueHandle_t am2301ResultQueue = xQueueCreate(2, sizeof(AM2301::result_t));
	if (am2301ResultQueue == 0) {
		ESP_LOGE(tag, "setup xQueueCreate failed (FATAL)");
		return;
	}

	LED led;
	led.setup(LED_GPIO, true);

	AM2301 am2301;
	am2301.setup(AM2301_GPIO, am2301ResultQueue);

	AM2301::result_t am2301Result;
	while (1) {

		led.signal(1);
		am2301.measure();
		if (xQueueReceive(am2301ResultQueue, &am2301Result,
				5000 / portTICK_PERIOD_MS)) {
			if (am2301Result.status == AM2301::result_status_t::RESULT_OK) {
				ESP_LOGI(tag, "T:%.1fC RH:%.1f%%",
						am2301Result.temperature - 273.15,
						am2301Result.humidity);
				led.signal(2);
			} else if (am2301Result.status
					== AM2301::result_status_t::RESULT_RECOVERABLE) {
				ESP_LOGE(tag, "app_main xQueueReceive RECOVERABLE");
				// continue
			} else if (am2301Result.status
					== AM2301::result_status_t::RESULT_FATAL) {
				ESP_LOGE(tag, "app_main xQueueReceive FATAL");
				// give up
				break;
			} else {
				ESP_LOGE(tag, "app_main status: %d", am2301Result.status);
			}
		}
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}

}
