// The author disclaims copyright to this source code.

#include "MHZ19B.h"

#include "string.h"

#include "driver/gpio.h"
#include "hal/uart_types.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "pubsub.h"

static const char *TAG = "MHZ19B";

const uint8_t MHZ19B::READ_CO2_CONCENTRATION_FRAME[] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 };
const uint8_t MHZ19B::SELF_CALIBRATION_OFF_FRAME[] = { 0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x96 };

MHZ19B::MHZ19B()
{
}

MHZ19B::~MHZ19B()
{
}

bool MHZ19B::initialize_uart()
{
    const uart_config_t uart_config = { //
            .baud_rate = 9600, //
                    .data_bits = UART_DATA_8_BITS, //
                    .parity = UART_PARITY_DISABLE, //
                    .stop_bits = UART_STOP_BITS_1, //
                    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, //
                    .rx_flow_ctrl_thresh = static_cast<uint8_t>(0), //
                    .source_clk = UART_SCLK_APB };

    // rx_buffer only read by task below
    rx_buffer = (uint8_t*) (malloc(RX_BUFFER_SIZE + 1));
    // no tx_buffer

    esp_err_t status = uart_driver_install(uart_port, RX_BUFFER_SIZE * 2, 0, 0, NULL, 0);
    if (status != ESP_OK) {
        ESP_LOGE(TAG, "initialize_uart, uart_driver_install (%s)", esp_err_to_name(status));
        return false;
    }
    status = uart_param_config(uart_port, &uart_config);
    if (status != ESP_OK) {
        ESP_LOGE(TAG, "initialize_uart, uart_param_config (%s)", esp_err_to_name(status));
        return false;
    }
    status = uart_set_pin(uart_port, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (status != ESP_OK) {
        ESP_LOGE(TAG, "initialize_uart, uart_set_pin (%s)", esp_err_to_name(status));
        return false;
    }
    return true;
}

void MHZ19B::setup(uart_port_t uart_port, gpio_num_t rx_pin, gpio_num_t tx_pin, const char *co2_topic,
        uint32_t measurement_period_ms)
{
    ESP_LOGD(TAG, "setup, uart_port:%d, rx_pin:%d, tx_pin:%d, topic_name:%s, this:%p", uart_port, rx_pin, tx_pin, co2_topic, this);
    this->uart_port = uart_port;
    this->rx_pin = rx_pin;
    this->tx_pin = tx_pin;
    this->co2_topic = co2_topic;

    // start task
    esp_err_t ret = xTaskCreate(&task, TAG, 2048, this, tskIDLE_PRIORITY,
    NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "setup, xTaskCreate failed:%d (FATAL)", ret);
        return;
    }
}

void MHZ19B::read_co2_concentration()
{
    ESP_LOGD(TAG, "read_co2_concentration");
    MHZ19B::write_frame(READ_CO2_CONCENTRATION_FRAME);
}

void MHZ19B::self_calibrate_off()
{
    ESP_LOGD(TAG, "self_calibrate_off");
    MHZ19B::write_frame(SELF_CALIBRATION_OFF_FRAME);
}

void MHZ19B::decode_frame(const uint8_t *frame)
{
    uint8_t expected_checksum = calculate_checksum(frame);
    if (frame[8] != expected_checksum) {
        ESP_LOGE(TAG, "decode_frame, %02x %02x %02x %02x %02x %02x %02x %02x %02x checksum failed", frame[0], frame[1], frame[2],
                frame[3], frame[4], frame[5], frame[6], frame[7], frame[8]);
    } else {
        uint8_t command = frame[1];
        if (command == 0x86) {
            decode_co2_concentration(frame);
        } else {
            ESP_LOGE(TAG, "decode_frame, %02x %02x %02x %02x %02x %02x %02x %02x %02x unexpected command", frame[0], frame[1],
                    frame[2], frame[3], frame[4], frame[5], frame[6], frame[7], frame[8]);
        }
    }
}

void MHZ19B::decode_co2_concentration(const uint8_t *frame)
{
    uint16_t ppm_co2 = (frame[2] << 8) + frame[3];
    ESP_LOGI(TAG, "decode_co2_concentration, co2:%d [ppm]", ppm_co2);
    pubsub_publish_double(co2_topic, (double) ppm_co2);
}

void MHZ19B::write_frame(const uint8_t *frame)
{
    int bytes_written = uart_write_bytes(uart_port, (const char*) frame, 9);
    if (bytes_written != 9) {
        ESP_LOGE(TAG, "write_frame, bytes_written: %d", bytes_written);
    }
}

void MHZ19B::write_frame_calculate(uint8_t *frame)
{
    uint8_t checksum = calculate_checksum(frame);
    frame[8] = checksum;
    write_frame(frame);
}

uint8_t MHZ19B::calculate_checksum(const uint8_t *frame)
{
    uint8_t checksum = 0;
    for (int i = 1; i < 8; i++) {
        checksum += frame[i];
    }
    checksum = 0xFF - checksum;
    checksum += 1;
    return checksum;
}

/**
 * Run task for this instance.
 */
void MHZ19B::run()
{
    ESP_LOGD(TAG, "run, this:%p", this);

    bool success = initialize_uart();
    if (!success) {
        ESP_LOGE(TAG, "run, initialize_uart failed (FATAL)");
    } else {
        self_calibrate_off();

        while (true) {
            const int rx_bytes = uart_read_bytes(uart_port, rx_buffer, RX_BUFFER_SIZE,
                    MINIMUM_MEASUREMENT_PERIOD_MS / portTICK_RATE_MS);
            if (rx_bytes == RX_BUFFER_SIZE) {
                decode_frame(rx_buffer);
            } else {
                ESP_LOGE(TAG, "run, %02x %02x %02x %02x %02x %02x %02x %02x %02x invalid frame length: %d", rx_buffer[0],
                        rx_buffer[1], rx_buffer[2], rx_buffer[3], rx_buffer[4], rx_buffer[5], rx_buffer[6], rx_buffer[7],
                        rx_buffer[8], rx_bytes);
            }
            vTaskDelay(measurement_period_ms / portTICK_PERIOD_MS);
        };
    }
}

/**
 * Link C static world to C++ instance
 */
void MHZ19B::task(void *pvParameter)
{
    if (pvParameter == 0) {
        ESP_LOGE(TAG, "task, invalid pvParameter");
    } else {
        // should be an instance
        MHZ19B *pInstance = (MHZ19B*) pvParameter;
        pInstance->run();
    }
}
