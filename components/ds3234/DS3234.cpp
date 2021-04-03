// The author disclaims copyright to this source code.

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
#include "freertos/timers.h"

#include "pubsub.h"

#include "DS3234.h"

/** Time update interval (ms) */
#define DS3432_LOOK_INTERVAL_MS 1000
/**
 * Limit for non-DMA SPI transfers.
 * Can not use DMA because need HALF DUPLEX transfers to avoid data corruption.
 */
#define MAX_TRANSFER_SIZE 64
/**
 * struct tm year epoch is 1900
 */
#define TM_YEAR_OFFSET 1900
/**
 * struct tm month is zero based
 */
#define TM_MONTH_OFFSET 1

/** Truncate dates to always be in this century 2000-01-01T00:00:00 */
#define TM_MINIMUM 946684800

#define TIME_REG 0x00
#define SRAM_ADDR_REG 0x18
#define SRAM_DATA_REG 0x19
#define REG_WRITE_BIT 0x80

static const char *TAG = "DS3234";

DS3234::DS3234()
{
}

DS3234::~DS3234()
{
}

void DS3234::encode_time(time_t time, uint8_t *raw)
{
    ESP_LOGD(TAG, "encode_time time:%ld", time);

    struct tm structured;
    gmtime_r(&time, &structured);

    ESP_LOGD(TAG, "encode_time structured:%d-%02d-%02d (%d) %02d:%02d:%02d", structured.tm_year + TM_YEAR_OFFSET,
            structured.tm_mon + TM_MONTH_OFFSET, structured.tm_mday, structured.tm_wday, structured.tm_hour, structured.tm_min,
            structured.tm_sec);

    raw[0] = int_to_bcd(structured.tm_sec);
    raw[1] = int_to_bcd(structured.tm_min);
    // can read 12 hour time
    // but will always set 24 hour
    raw[2] = int_to_bcd(structured.tm_hour);
    raw[3] = int_to_bcd(structured.tm_wday);
    raw[4] = int_to_bcd(structured.tm_mday);
    // already in century 20xx
    // tm_mon is 0-based month number
    raw[5] = int_to_bcd(structured.tm_mon + 1) | 0x80;
    raw[6] = int_to_bcd(structured.tm_year % 100);

    ESP_LOGD(TAG, "encode_time raw:%02X %02X %02X %02X %02X %02X %02X", raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6]);

}

time_t DS3234::decode_time(const uint8_t *raw)
{
    ESP_LOGD(TAG, "decode_time raw:%02X %02X %02X %02X %02X %02X %02X", raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6]);

    struct tm structured;
    structured.tm_sec = bcd_to_int(raw[0]);
    structured.tm_min = bcd_to_int(raw[1]);
    // can read 12 hour time
    // but will always set 24 hour
    structured.tm_hour = hour_to_int(raw[2]);
    structured.tm_wday = bcd_to_int(raw[3]);
    structured.tm_mday = bcd_to_int(raw[4]);
    // month number (exclude century bit)
    uint8_t month = bcd_to_int(raw[5] & 0x7F);
    // tm_mon is 0-based month number
    structured.tm_mon = month - 1;
    // tm_year = 1900 based year (always in 20xx)
    structured.tm_year = bcd_to_int(raw[6]) + 100;

    ESP_LOGD(TAG, "decode_time structured:%d-%02d-%02d (%d) %02d:%02d:%02d", structured.tm_year + TM_YEAR_OFFSET,
            structured.tm_mon + TM_MONTH_OFFSET, structured.tm_mday, structured.tm_wday, structured.tm_hour, structured.tm_min,
            structured.tm_sec);

    time_t time = mktime(&structured);
    ESP_LOGD(TAG, "decode_time time:%ld", time);
    return time;
}

uint8_t DS3234::bcd_to_int(uint8_t bcd)
{
    return bcd - 6 * (bcd >> 4);
}

uint8_t DS3234::int_to_bcd(uint8_t dec)
{
    return dec + 6 * (dec / 10);
}

uint8_t DS3234::hour_to_int(uint8_t bcdHour)
{
    uint8_t hour;
    bool is12 = bcdHour & 0x40;
    if (is12) {
        bool isPm = bcdHour & 0x20;
        hour = bcd_to_int(bcdHour & 0x1f);
        if (isPm) {
            hour += 12;
        }
    } else {
        hour = bcd_to_int(bcdHour);
    }
    return hour;
}

/**
 * Link C static world to C++ instance
 */
void DS3234::task(void *pvParameter)
{
    if (pvParameter == 0) {
        ESP_LOGE(TAG, "task, invalid pvParameter");
    } else {
        // should be an instance
        DS3234 *pInstance = (DS3234*) pvParameter;
        pInstance->run();
    }
}

void DS3234::setup(pubsub_topic_t topic, const char *topic_name)
{
    ESP_LOGD(TAG, "setup, topic:%p, topic_name:%s, this:%p", topic, topic_name, this);

    this->timestamp_topic = topic;

    tx = malloc(MAX_TRANSFER_SIZE);
    if (tx == NULL) {
        ESP_LOGE(TAG, "run malloc tx failed (FATAL)");
        return;
    }
    rx = malloc(MAX_TRANSFER_SIZE);
    if (rx == NULL) {
        ESP_LOGE(TAG, "run malloc rx failed (FATAL)");
        return;
    }

    // when time set actions arrive fast
    time_queue = xQueueCreate(10, sizeof(pubsub_message_t));
    pubsub_add_subscription(time_queue, topic_name, false);

    // start task
    esp_err_t ret = xTaskCreate(&task, TAG, 3072, this, tskIDLE_PRIORITY,
    NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "setup xTaskCreate failed:%d (FATAL)", ret);
        return;
    }
}

void DS3234::write_data(const uint8_t cmd, const uint8_t *data, const int len)
{
    ESP_LOGD(TAG, "writeData cmd:%02X, len:%d", cmd, len);
    assert(len <= MAX_TRANSFER_SIZE);
    memcpy(tx, data, len);

    spi_transaction_t transaction;
    memset(&transaction, 0, sizeof(spi_transaction_t));
    transaction.flags = 0;
    transaction.cmd = cmd | REG_WRITE_BIT;
    transaction.addr = 0;
    transaction.length = len * 8;
    transaction.rxlength = 0;
    transaction.tx_buffer = tx;
    transaction.rx_buffer = NULL;

    spi_device_acquire_bus(device_handle, portMAX_DELAY);
    gpio_set_level(GPIO_NUM_15, 0);
    esp_err_t ret = spi_device_transmit(device_handle, &transaction);
    gpio_set_level(GPIO_NUM_15, 1);
    assert(ret==ESP_OK);
    spi_device_release_bus(device_handle);
}

void DS3234::read_data(const uint8_t cmd, uint8_t *data, const int len)
{
    ESP_LOGD(TAG, "read_data cmd:%02X, len:%d", cmd, len);
    assert(len <= MAX_TRANSFER_SIZE);

    spi_transaction_t transaction;
    memset(&transaction, 0, sizeof(spi_transaction_t));
    transaction.flags = 0;
    transaction.cmd = cmd;
    transaction.addr = 0;
    transaction.length = 0;
    transaction.rxlength = len * 8;
    transaction.tx_buffer = NULL;
    transaction.rx_buffer = rx;

    spi_device_acquire_bus(device_handle, portMAX_DELAY);
    gpio_set_level(GPIO_NUM_15, 0);
    esp_err_t ret = spi_device_transmit(device_handle, &transaction);
    gpio_set_level(GPIO_NUM_15, 1);
    assert(ret==ESP_OK);
    spi_device_release_bus(device_handle);

    memcpy(data, rx, len);
}

bool DS3234::self_test()
{
    uint8_t addr = 0;
    uint8_t data[MAX_TRANSFER_SIZE];
    // write
    for (int i = 0; i < MAX_TRANSFER_SIZE; i++) {
        data[i] = i;
    }
    write_data(SRAM_ADDR_REG, &addr, 1);
    write_data(SRAM_DATA_REG, data, MAX_TRANSFER_SIZE);
    // scrub
    memset(data, 0, MAX_TRANSFER_SIZE);
    // read
    write_data(SRAM_ADDR_REG, &addr, 1);
    read_data(SRAM_DATA_REG, data, MAX_TRANSFER_SIZE);
    // compare
    bool success = true;
    for (int i = 0; i < MAX_TRANSFER_SIZE; i++) {
        if (data[i] != i) {
            success = false;
            ESP_LOGE(TAG, "self_test failed [%d/%d]", i, MAX_TRANSFER_SIZE);
            break;
        }
    }
    if (success) {
        ESP_LOGD(TAG, "self_test OK");
    }
    return success;
}

/**
 * Run task for this instance.
 */
void DS3234::run()
{
    ESP_LOGD(TAG, "run, this:%p", this);

    // configure spi bus
    spi_bus_config_t buscfg;
    memset(&buscfg, 0, sizeof(spi_bus_config_t));
    buscfg.flags = SPICOMMON_BUSFLAG_IOMUX_PINS;
    buscfg.miso_io_num = GPIO_NUM_12;
    buscfg.mosi_io_num = GPIO_NUM_13;
    buscfg.sclk_io_num = GPIO_NUM_14;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = MAX_TRANSFER_SIZE;
    esp_err_t ret = spi_bus_initialize(HSPI_HOST, &buscfg, 0);
    ESP_ERROR_CHECK(ret);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "run spi_bus_initialize failed (FATAL)");
        return;
    }

    // configure spi device (READ)
    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(spi_device_interface_config_t));
    devcfg.command_bits = 8;
    devcfg.mode = 3;
    devcfg.clock_speed_hz = 1000000;
    devcfg.spics_io_num = -1;
    devcfg.flags = SPI_DEVICE_HALFDUPLEX;
    devcfg.dummy_bits = 0;
    devcfg.queue_size = 1;
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &device_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "run spi_bus_add_device (READ) failed (FATAL)");
        return;
    }

    // manually drive CS to create extra time before first CLK signal
    // DS3234 determines SPI mode depending on level at CS start.
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_15);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "run gpio_config failed:%d (FATAL)", ret);
        return;
    }

    gpio_set_drive_capability(GPIO_NUM_15, GPIO_DRIVE_CAP_0);
    gpio_set_level(GPIO_NUM_15, 1);

    if (self_test() == false) {
        ESP_LOGE(TAG, "run self test failed (FATAL)");
        return;
    }

    // periodic listen for set time changes and publish time updates
    uint8_t raw[] = { 0, 0, 0, 0, 0, 0, 0 };
    time_t time = -1;
    pubsub_message_t message;
    while (true) {

        if (xQueueReceive(time_queue, &message, DS3432_LOOK_INTERVAL_MS / portTICK_PERIOD_MS)) {

            // truncate incoming date
            if (message.int_val < TM_MINIMUM) {
                time = TM_MINIMUM;
            }
            // ignore own publish action
            if (time != message.int_val) {
                ESP_LOGD(TAG, "run set time");
                encode_time(message.int_val, raw);
                write_data(TIME_REG, raw, 7);
            }
        } else {

            ESP_LOGD(TAG, "run read time");
            read_data(TIME_REG, raw, 7);
            time = decode_time(raw);
            pubsub_publish_int(timestamp_topic, time);
        }
    };
}

