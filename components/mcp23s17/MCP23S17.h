// The author disclaims copyright to this source code.

#ifndef _MCP23S17_H_
#define _MCP23S17_H_

#include "hal/spi_types.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "pubsub.h"

/**
 * MCP23S17 16-bit I/O expander.<br>
 * - used for output only
 * - used in IOCON mode 0
 */
class MCP23S17
{

public:
    MCP23S17();
    virtual ~MCP23S17();
    /**
     * Setup once before use.
     *
     * @param host_id SPI host id
     * @param cs_pin chip select pin number
     * @param address address of device [0..7]
     * @param topics list of topics [16]
     */
    void setup(spi_host_device_t host_id, gpio_num_t cs_pin, uint8_t address, const char *topics[16]);

private:
    const uint8_t BASE_ADDRESS = 0x40;
    const uint8_t IODIRA = 0x00;
    const uint8_t IODIRB = 0x01;
    const uint8_t IOCON = 0x0A;
    const uint8_t GPIOA = 0x12;
    const uint8_t GPIOB = 0x13;

    const uint16_t BIT_ON[16] = { //
            0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, //
                    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000 };
    const uint16_t BIT_OFF[16] = { //
            0xFFFE, 0xFFFD, 0xFFFB, 0xFFF7, 0xFFEF, 0xFFDF, 0xFFBF, 0xFF7F, //
                    0xFEFF, 0xFDFF, 0xFBFF, 0xF7FF, 0xEFFF, 0xDFFF, 0xBFFF, 0x7FFF };

    spi_host_device_t host_id = (spi_host_device_t) -1;
    gpio_num_t cs_pin = GPIO_NUM_NC;

    uint8_t address = 0;
    uint16_t output_state = 0;

    /**
     * Handle to SPI device.
     */
    spi_device_handle_t device_handle = 0;

    QueueHandle_t queue = 0;

    const char *topics[16];

    void init_topics(const char *topics[]);
    void write_byte(const uint8_t reg, const uint8_t value);
    void write_word(const uint8_t reg, const uint16_t value);
    uint32_t read_word(const uint8_t reg);

    /**
     * Run task for this instance.
     */
    void run();

    /**
     * Link C static world to C++ instance
     */
    static void task(void *pvParameter);

};

#endif /* _MCP23S17_H_ */
