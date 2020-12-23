// The author disclaims copyright to this source code.
#ifndef _LED_H_
#define _LED_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

/**
 * Signal LED.
 * Blinks a LED to signal states.
 */
class LED {

public:
	LED();
	virtual ~LED();
	/**
	 * Setup once before use.
	 * @param pin one wire output pin
	 * @param on level used to light LED
	 */
	void setup(gpio_num_t pin, bool on);
	void signal(uint8_t count);

private:
	gpio_num_t pin = GPIO_NUM_NC;
	bool on = true;
	xQueueHandle queue = 0;

	void run();

	static void task(void *pvParameter);
};

#endif /* _LED_H_ */
