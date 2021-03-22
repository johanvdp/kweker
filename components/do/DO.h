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
	 * @param active_high true if active high output
	 * @param topic topic receiving messages
	 */
	void setup(gpio_num_t pin, bool active_high, const char* topic);

private:
	gpio_num_t pin = GPIO_NUM_NC;
	bool active_high = true;
	QueueHandle_t queue = 0;

	void run();
	void write(bool on);

	static void task(void *pvParameter);
};

#endif /* _DO_H_ */
