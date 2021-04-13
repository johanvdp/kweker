// The author disclaims copyright to this source code.

#ifndef _DS3234_H_
#define _DS3234_H_

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "pubsub.h"

/**
 * DS3234 time.
 */
class DS3234
{

public:
    DS3234();
    virtual ~DS3234();
    /**
     * Setup once before use.
     * @param time_topic time topic, used to publish and subscribe to time changes.
     */
    void setup(const char *time_topic);

private:

    gpio_num_t cs_pin = GPIO_NUM_NC;

    /**
     * Timestamp topic. Publish ticks since epoch.
     */
    const char *timestamp_topic = 0;

    /**
     * Handle to SPI device.
     */
    spi_device_handle_t device_handle = 0;

    /** set time queue */
    QueueHandle_t time_queue = 0;

    void *tx = 0;
    void *rx = 0;
    /**
     * Run task for this instance.
     */
    void run();

    /**
     * Run self test. Erases memory.
     * @return true when OK.
     */
    bool self_test();

    void write_data(const uint8_t cmd, const uint8_t *data, const int len);
    void read_data(const uint8_t cmd, uint8_t *data, const int len);

    /**
     * From time to raw time.
     */
    void encode_time(time_t time, uint8_t *raw);
    /**
     * From raw time to time.
     */
    time_t decode_time(const uint8_t *raw);

    uint8_t bcd_to_int(uint8_t bcd);
    uint8_t int_to_bcd(uint8_t dec);
    uint8_t hour_to_int(uint8_t bcd);

    /**
     * Link C static world to C++ instance
     */
    static void task(void *pvParameter);
};

#endif /* _DS3234_H_ */
