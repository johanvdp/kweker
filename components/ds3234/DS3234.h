// The author disclaims copyright to this source code.
#ifndef _DS3234_H_
#define _DS3234_H_

#include "pubsub.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

/**
 * DS3234 time.
 */
class DS3234 {

public:
    DS3234();
    virtual ~DS3234();
    /**
     * Setup once before use.
     * @param timestamp_topic timestamp topic.
     */
    void setup(pubsub_topic_t timestamp_topic);

private:

    gpio_num_t cs_pin = GPIO_NUM_NC;

    /**
     * Timestamp topic. Publish ticks since epoch.
     */
    pubsub_topic_t timestamp_topic = 0;
    /**
     * Handle to SPI device.
     */
    spi_device_handle_t device_handle = 0;

    void *tx;
    void *rx;
    void run();

    /**
     * Run self test. Erases memory.
     * @return true when OK.
     */
    bool selfTest();

    void writeData(const uint8_t cmd, const uint8_t *data, const int len);
    void readData(const uint8_t cmd, uint8_t *data, const int len);

    void decodeRawTime(const uint8_t *raw, struct tm *structured);
    uint8_t bcdToInt(uint8_t bcd);
    uint8_t intToBcd(uint8_t dec);
    uint8_t bcdTo24Hour(uint8_t bcdHour);

    /** periodic timer task */
    static void task(TimerHandle_t xTimer);
};

#endif /* _DS3234_H_ */
