// The author disclaims copyright to this source code.

#ifndef _MHZ19B_H_
#define _MHZ19B_H_

#include "hal/gpio_types.h"
#include "hal/uart_types.h"

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
     * @param uart_port UART port number
     * @param rx_pin RX pin number
     * @param tx_pin TX pin number
     * @param co2_topic CO2 concentration topic [ppm].
     * @param measurement_period_ms measurement period [ms].
     */
    void setup(uart_port_t uart_port, gpio_num_t rx_pin, gpio_num_t tx_pin, const char *co2_topic, uint32_t measurement_period_ms);

    /** Minimum measurement period [ms]. */
    static constexpr int MINIMUM_MEASUREMENT_PERIOD_MS = 120000;
private:

    uart_port_t uart_port = 0;
    gpio_num_t rx_pin = GPIO_NUM_NC;
    gpio_num_t tx_pin = GPIO_NUM_NC;

    /**
     * CO2 concentration topic [ppm].
     */
    const char *co2_topic = 0;

    /**
     * Measurement period [ms].
     */
    uint32_t measurement_period_ms = MINIMUM_MEASUREMENT_PERIOD_MS;

    /**
     * Prepared command: read CO2 concentration (reply expected)
     * byte0: start    0xFF
     * byte1: reserved 0x01
     * byte2: command  0x86
     * byte3..7: -     0x00
     * byte8: checksum 0x79
     */
    static const uint8_t READ_CO2_CONCENTRATION_FRAME[];
    /**
     * Prepared command: self calibrate off (no reply expected)
     * byte0: start    0xFF
     * byte1: reserved 0x01
     * byte2: command  0x79
     * byte3..7: -     0x00
     * byte8: checksum 0x86
     */
    static const uint8_t SELF_CALIBRATION_OFF_FRAME[];

    /** Size of command/response frame. */
    static const int FRAME_LENGTH = 9;
    /** At least frame length but larger than UART_FIFO_LEN. */
    static const int RX_BUFFER_SIZE = UART_FIFO_LEN + 1;
    /** Buffer to receive one frame. */
    uint8_t *rx_buffer = 0;

    /**
     * Initialize UART
     *
     * @return true if successful
     */
    bool initialize_uart();

    /**
     * Command: read CO2 concentration.
     * Expect reply: read CO2 concentration.
     */
    void command_read_co2_concentration();

    /**
     * Command: self calibrate off.
     * Expect reply: none
     */
    void command_self_calibrate_off();

    /**
     * Write frame to device.
     *
     * @param frame The frame.
     */
    void write_frame(const uint8_t *frame);

    /**
     * Write frame to device.
     * Modifies the frame to add the checksum.
     *
     * @param frame The frame.
     */
    void write_frame_calculate(uint8_t *frame);

    /**
     * Decode received frame.
     *
     * @param frame The frame.
     */
    void decode_frame(const uint8_t *frame);

    /**
     * Decode co2 concentration frame.
     *
     * @param frame The frame.
     */
    void decode_co2_concentration(const uint8_t *frame);

    /**
     * Calculate the checksum of the data (7 bytes) in a frame (9 bytes).
     * That is; excluding byte 0 (start), and excluding byte 8 (checksum).
     * checksum = negative(byte 1 + byte 2 + .. + byte 7) + 1.
     *
     * @param frame the frame
     * @return the checksum
     */
    uint8_t calculate_checksum(const uint8_t *bytes);

    /**
     * Write task for this instance.
     */
    void write();
    /**
     * Read task for this instance.
     */
    void read();

    /**
     * Link C static world to C++ instance
     */
    static void read_task(void *pvParameter);

    /**
     * Link C static world to C++ instance
     */
    static void write_task(void *pvParameter);
};

#endif /* _MHZ19B_H_ */
