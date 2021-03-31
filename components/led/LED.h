// The author disclaims copyright to this source code.

#ifndef _LED_H_
#define _LED_H_

#include "driver/gpio.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pubsub.h"

/**
 * Signal LED.
 * Blinks a LED to signal states.
 */
class LED
{

public:
    LED();
    virtual ~LED();
    /**
     * Setup once before use.
     * @param pin one wire output pin
     * @param on level used to light LED
     * @param topic topic receiving messages
     */
    void setup(gpio_num_t pin, bool on, const char *topic);

private:
    gpio_num_t pin = GPIO_NUM_NC;
    bool on = true;
    QueueHandle_t queue = 0;

    void run();

    static void task(void *pvParameter);
};

#endif /* _LED_H_ */
