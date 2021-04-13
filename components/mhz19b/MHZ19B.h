// The author disclaims copyright to this source code.

#ifndef _MHZ19B_H_
#define _MHZ19B_H_

#include "driver/gpio.h"

#include "pubsub.h"

/**
 * MHZ19B CO2 concentration module.
 */
class MHZ19B
{

public:
    MHZ19B();
    virtual ~MHZ19B();
    /**
     * Setup once before use.
     *
     * @param rx_pin RX pin number
     * @param tx_pin TX pin number
     * @param co2_topic CO2 concentration topic [ppm].
     */
    void setup(gpio_num_t rx_pin, gpio_num_t tx_pin, const char *co2_topic);

private:

    gpio_num_t rx_pin = GPIO_NUM_NC;
    gpio_num_t tx_pin = GPIO_NUM_NC;

    /**
     * CO2 concentration topic [ppm].
     */
    const char *co2_topic = 0;

    /**
     * Run task for this instance.
     */
    void run();
    /**
     * Link C static world to C++ instance
     */
    static void task(void *pvParameter);
};

#endif /* _MHZ19B_H_ */
