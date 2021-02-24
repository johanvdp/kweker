// The author disclaims copyright to this source code.
#ifndef _DO_H_
#define _DO_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "pubsub.h"

/**
 * Digital output.
 */
class DO {

public:
    DO();
	virtual ~DO();
	/**
	 * Setup once before use.
	 * @param pin one wire output pin
	 * @param on level used as on
	 * @param topic topic receiving messages
	 */
	void setup(gpio_num_t pin, bool on, const char* topic);

private:
	gpio_num_t pin = GPIO_NUM_NC;
	bool on = true;
	QueueHandle_t queue = 0;

	void run();

	static void task(void *pvParameter);
};

#endif /* _DO_H_ */
