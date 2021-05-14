// The author disclaims copyright to this source code.

#include "MCP23S17.h"

#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "hal/gpio_types.h"
#include "driver/spi_master.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "pubsub.h"

static const char *TAG = "MCP23S17";

MCP23S17::MCP23S17()
{
}

MCP23S17::~MCP23S17()
{
}

/**
 * Link C static world to C++ instance
 */
void MCP23S17::task(void *pvParameter)
{
    if (pvParameter == 0) {
        ESP_LOGE(TAG, "task, invalid pvParameter");
    } else {
        // should be an instance
        MCP23S17 *pInstance = (MCP23S17*) pvParameter;
        pInstance->run();
    }
}

void MCP23S17::setup(spi_host_device_t host_id, gpio_num_t cs_pin, uint8_t address, const char *topics[16])
{
    ESP_LOGD(TAG, "setup, host_id:%d, cs_pin:%d, address:%02X, topics:%p, this:%p", host_id, cs_pin, address, topics, this);

    this->host_id = host_id;
    this->cs_pin = cs_pin;
    this->address = address;

    // configure spi device
    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(spi_device_interface_config_t));
    devcfg.mode = 0;
    devcfg.clock_speed_hz = 1000000;
    devcfg.spics_io_num = cs_pin;
    devcfg.flags = SPI_DEVICE_HALFDUPLEX;
    devcfg.queue_size = 1;
    esp_err_t ret = spi_bus_add_device(host_id, &devcfg, &device_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "run, spi_bus_add_device failed (FATAL)");
        return;
    }

    // program MCP23S17 registers
    // all set to defaults after RESET signal
    // except:
    //
    // IOCON[7..0]: BANK MIRROR SEQOP DISSLW HAEN ODR INTPOL reserved
    //              HAEN = host address enabled
    write_byte(IOCON, 0x08);

    // create queue and connect output bit topics
    queue = xQueueCreate(64, sizeof(pubsub_message_t));
    init_topics(topics);

    // start task
    ret = xTaskCreate(&task, TAG, 3072, this, tskIDLE_PRIORITY,
    NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "setup, xTaskCreate failed:%d (FATAL)", ret);
        return;
    }
}

void MCP23S17::init_topics(const char *topic_list[])
{
    // all bits initially inputs (1)
    uint16_t input_bits = 0xFFFF;
    // scan all topicss
    for (int topic_index = 0; topic_index < 16; topic_index++) {
        if (topic_list[topic_index] == NULL) {
            // topic not present
            topics[topic_index] = NULL;
        } else {
            // topic present (clone it)
            topics[topic_index] = strdup(topic_list[topic_index]);
            // subscribe to it
            pubsub_add_subscription(queue, topics[topic_index], true);
            // and make the bit an output (0)
            input_bits &= BIT_OFF[topic_index];
        }
    }

    // write IODIRA + IODIRB[7..0]: I/O direction
    write_word(IODIRA, input_bits);
}

void MCP23S17::write_byte(const uint8_t reg, const uint8_t value)
{
    ESP_LOGD(TAG, "write_byte, reg:%02X, value:%02X", reg, value);

    spi_transaction_t transaction;
    memset(&transaction, 0, sizeof(spi_transaction_t));
    transaction.flags = SPI_TRANS_USE_TXDATA;
    transaction.length = 24;
    // write R/W = 0
    transaction.tx_data[0] = BASE_ADDRESS | (address << 1);
    transaction.tx_data[1] = reg;
    transaction.tx_data[2] = value;

    esp_err_t ret = spi_device_transmit(device_handle, &transaction);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "write_byte, spi_device_transmit failed:%d", ret);
        return;
    }
}

void MCP23S17::write_word(const uint8_t reg, const uint16_t value)
{
    ESP_LOGD(TAG, "write_word, reg:%02X, value:%04X", reg, value);

    spi_transaction_t transaction;
    memset(&transaction, 0, sizeof(spi_transaction_t));
    transaction.flags = SPI_TRANS_USE_TXDATA;
    transaction.length = 32;
    // write R/W = 0
    transaction.tx_data[0] = BASE_ADDRESS | (address << 1);
    transaction.tx_data[1] = reg;
    transaction.tx_data[2] = (uint8_t) (value);
    transaction.tx_data[3] = (uint8_t) (value >> 8);

    esp_err_t ret = spi_device_transmit(device_handle, &transaction);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "write_word, spi_device_transmit failed:%d", ret);
        return;
    }
}

uint32_t MCP23S17::read_word(const uint8_t reg)
{
    ESP_LOGD(TAG, "read_word, reg:%02X", reg);

    spi_transaction_t transaction;
    memset(&transaction, 0, sizeof(spi_transaction_t));
    transaction.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
    transaction.length = 32;
    // write R/W = 1
    transaction.tx_data[0] = BASE_ADDRESS | (address << 1) | 0x01;
    transaction.tx_data[1] = reg;
    transaction.tx_data[2] = 0x00;
    transaction.tx_data[3] = 0x00;

    esp_err_t ret = spi_device_transmit(device_handle, &transaction);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "write_word, spi_device_transmit failed:%d", ret);
        return -1;
    }
    ESP_LOGI(TAG, "read_word, %02X %02X %02X %02X", transaction.rx_data[0], transaction.rx_data[1], transaction.rx_data[2],
            transaction.rx_data[3]);
    return ((transaction.rx_data[0] << 8) | transaction.rx_data[1]);
}

/**
 * Run task for this instance.
 */
void MCP23S17::run()
{
    ESP_LOGD(TAG, "run, this:%p", this);

    // wait for output messages
    pubsub_message_t message;
    while (true) {
        while (xQueueReceive(queue, &message, portMAX_DELAY)) {
            for (int topic_index = 0; topic_index < 16; topic_index++) {
                if (strcmp(message.topic, topics[topic_index]) == 0) {
                    // found
                    bool value = message.boolean_val;
                    if (value) {
                        output_state |= BIT_ON[topic_index];
                    } else {
                        output_state &= BIT_OFF[topic_index];
                    }
                    // write GPIOA + GPIOB
                    write_word(GPIOA, output_state);
                }
            }
        };
    };
}

