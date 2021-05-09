// The author disclaims copyright to this source code.

#include "MHZ19B.h"

#include "string.h"

#include "driver/gpio.h"
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
    // somehow using single constructor does not work
    uart_config_t uart_config;
    uart_config.baud_rate = 9600;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.rx_flow_ctrl_thresh = 0;
    uart_config.source_clk = UART_SCLK_APB;

    // rx_buffer only read by task below
    rx_buffer = (uint8_t*) (malloc(RX_BUFFER_SIZE));
    // no tx_buffer

    esp_err_t status = uart_driver_install(uart_port, RX_BUFFER_SIZE, 0, 0, NULL, 0);
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
    status = uart_set_loop_back(uart_port, false);
    if (status != ESP_OK) {
        ESP_LOGE(TAG, "initialize_uart, uart_set_loop_back (%s)", esp_err_to_name(status));
        return false;
    }

    return true;
}

void MHZ19B::setup(uart_port_t uart_port, gpio_num_t rx_pin, gpio_num_t tx_pin, const char *co2_topic,
        uint32_t measurement_period_ms)
{
    ESP_LOGI(TAG, "setup, uart_port:%d, rx_pin:%d, tx_pin:%d, topic_name:%s, this:%p", uart_port, rx_pin, tx_pin, co2_topic, this);
    this->uart_port = uart_port;
    this->rx_pin = rx_pin;
    this->tx_pin = tx_pin;
    this->co2_topic = co2_topic;
    this->measurement_period_ms = measurement_period_ms;

    bool success = initialize_uart();
    if (!success) {arguments
        ESP_LOGE(TAG, "write, initialize_uart failed (FATAL)");
        return;
    }

    // start read task
    esp_err_t status = xTaskCreate(&read_task, TAG, 3072, this, tskIDLE_PRIORITY, NULL);
    if (status != pdPASS) {
        ESP_LOGE(TAG, "setup, xTaskCreate read_task failed:%d (FATAL)", status);
        return;
    }

    // start write task
    status = xTaskCreate(&write_task, TAG, 3072, this, tskIDLE_PRIORITY, NULL);
    if (status != pdPASS) {
        ESP_LOGE(TAG, "setup, xTaskCreate write_task failed:%d (FATAL)", status);
        return;
    }
}

void MHZ19B::command_read_co2_concentration()
{
    ESP_LOGI(TAG, "command_read_co2_concentration");
    MHZ19B::write_frame(READ_CO2_CONCENTRATION_FRAME);
}

void MHZ19B::command_self_calibrate_off()
{
    ESP_LOGI(TAG, "command_self_calibrate_off");
    MHZ19B::write_frame(SELF_CALIBRATION_OFF_FRAME);
}

void MHZ19B::decode_frame(const uint8_t *frame)
{
    uint8_t start_byte = frame[0];
    if (start_byte != 0xFF) {
        ESP_LOGE(TAG, "decode_frame, %02x %02x %02x %02x %02x %02x %02x %02x %02x invalid frame format", frame[0], frame[1],
                frame[2], frame[3], frame[4], frame[5], frame[6], frame[7], frame[8]);
        return;
    }
    uint8_t expected_checksum = calculate_checksum(frame);
    if (frame[8] != expected_checksum) {
        ESP_LOGE(TAG, "decode_frame, %02x %02x %02x %02x %02x %02x %02x %02x %02x checksum failed", frame[0], frame[1], frame[2],
                frame[3], frame[4], frame[5], frame[6], frame[7], frame[8]);
        return;
    }
    uint8_t command = frame[1];
    if (command == 0x86) {
        decode_co2_concentration(frame);
    } else {
        ESP_LOGE(TAG, "decode_frame, %02x %02x %02x %02x %02x %02x %02x %02x %02x unexpected command", frame[0], frame[1], frame[2],
                frame[3], frame[4], frame[5], frame[6], frame[7], frame[8]);
        return;
    }
}

void MHZ19B::decode_co2_concentration(const uint8_t *frame)
{
    uint8_t ppm_hi = frame[2];
    uint8_t ppm_lo = frame[3];
    uint16_t ppm_co2 = ppm_hi * 256 + ppm_lo;
    ESP_LOGI(TAG, "decode_co2_concentration, %02x %02x", ppm_hi, ppm_lo);
    ESP_LOGI(TAG, "decode_co2_concentration, co2:%d [ppm]", ppm_co2);
    pubsub_publish_double(co2_topic, (double) ppm_co2);
}

void MHZ19B::write_frame(const uint8_t *frame)
{
    ESP_LOGI(TAG, "write_frame, %02x %02x %02x %02x %02x %02x %02x %02x %02x", frame[0], frame[1], frame[2], frame[3], frame[4],
            frame[5], frame[6], frame[7], frame[8]);

    int bytes_written = uart_write_bytes(uart_port, (const char*) frame, FRAME_LENGTH);
    if (bytes_written != FRAME_LENGTH) {
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
 * Run read task for this instance.
 */
void MHZ19B::read()
{
    ESP_LOGI(TAG, "read, this:%p", this);

    while (true) {
        ESP_LOGI(TAG, "read, uart_read_bytes");
        const int rx_bytes = uart_read_bytes(uart_port, rx_buffer, FRAME_LENGTH,  portMAX_DELAY);
        if (rx_bytes == -1) {
            ESP_LOGE(TAG, "read, error");
        } else if (rx_bytes == 0) {
            ESP_LOGI(TAG, "read, timeout");
        } else if (rx_bytes == FRAME_LENGTH) {
            decode_frame(rx_buffer);
        } else {
            ESP_LOGI(TAG, "read, %02x %02x %02x %02x %02x %02x %02x %02x %02x invalid frame length: %d", rx_buffer[0], rx_buffer[1],
                    rx_buffer[2], rx_buffer[3], rx_buffer[4], rx_buffer[5], rx_buffer[6], rx_buffer[7], rx_buffer[8], rx_bytes);
            // remove left over of malformed frame
            esp_err_t status = uart_flush(uart_port);
            if (status != ESP_OK) {
                ESP_LOGE(TAG, "read, uart_flush (%s)", esp_err_to_name(status));
            }
        }
    };
}

/**
 * Run write task for this instance.
 */
void MHZ19B::write()
{
    ESP_LOGI(TAG, "write, this:%p", this);

    while (true) {
        command_read_co2_concentration();
        vTaskDelay(measurement_period_ms / portTICK_PERIOD_MS);
    };
}

/**
 * Link C static world to C++ instance
 */
void MHZ19B::read_task(void *pvParameter)
{
    if (pvParameter == 0) {
        ESP_LOGE(TAG, "read_task, invalid pvParameter");
    } else {
        // should be an instance
        MHZ19B *pInstance = (MHZ19B*) pvParameter;
        pInstance->read();
    }
}

/**
 * Link C static world to C++ instance
 */
void MHZ19B::write_task(void *pvParameter)
{
    if (pvParameter == 0) {
        ESP_LOGE(TAG, "write_task, invalid pvParameter");
    } else {
        // should be an instance
        MHZ19B *pInstance = (MHZ19B*) pvParameter;
        pInstance->write();
    }
}
