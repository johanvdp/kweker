// The author disclaims copyright to this source code.
#include "DS3234.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "driver/spi_master.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_err.h"
#include "hal/gpio_types.h"
#include "time.h"
#include "pubsub.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"

/** Clock update interval (ms) */
#define DS3432_LOOK_INTERVAL_MS 1000
#define MAX_TRANSFER_SIZE 64
#define TEST_BYTES 1

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

void DS3234::decodeRawTime(const uint8_t *raw, struct tm *structured)
{
    structured->tm_sec = bcdToInt(raw[0]);
    structured->tm_min = bcdToInt(raw[1]);
    structured->tm_hour = bcdTo24Hour(raw[2]);
    structured->tm_wday = bcdToInt(raw[3]);
    structured->tm_mday = bcdToInt(raw[4]);
    structured->tm_mon = bcdToInt(raw[5]);
    structured->tm_year = bcdToInt(raw[6]);
}

uint8_t DS3234::bcdToInt(uint8_t bcd)
{
    return bcd - 6 * (bcd >> 4);
}

uint8_t DS3234::intToBcd(uint8_t dec)
{
    return dec + 6 * (dec / 10);
}

uint8_t DS3234::bcdTo24Hour(uint8_t bcdHour)
{
    uint8_t hour;
    if (bcdHour & 0x40) {
        bool isPm = ((bcdHour & 0x20) != 0);
        hour = bcdToInt(bcdHour & 0x1f);
        if (isPm) {
            hour += 12;
        }
    } else {
        hour = bcdToInt(bcdHour);
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

void DS3234::setup(pubsub_topic_t topic)
{
    ESP_LOGI(TAG, "setup, topic:%p, this:%p", topic, this);

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

    // start periodic task
    esp_err_t ret = xTaskCreate(&task, "setup", 4096, this, tskIDLE_PRIORITY,
    NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "setup xTaskCreate failed:%d (FATAL)", ret);
        return;
    }
}

void DS3234::writeData(const uint8_t cmd, const uint8_t *data, const int len)
{
    ESP_LOGD(TAG, "writeData cmd:%02X, len:%d", cmd, len);
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

void DS3234::readData(const uint8_t cmd, uint8_t *data, const int len)
{
    ESP_LOGD(TAG, "readData cmd:%02X, len:%d", cmd, len);
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

bool DS3234::selfTest()
{
    uint8_t addr = 0;
    uint8_t data[MAX_TRANSFER_SIZE];
    // write
    for (int i = 0; i < MAX_TRANSFER_SIZE; i++) {
        data[i] = i;
    }
    writeData(SRAM_ADDR_REG, &addr, 1);
    writeData(SRAM_DATA_REG, data, MAX_TRANSFER_SIZE);
    // scrub
    memset(data, 0, MAX_TRANSFER_SIZE);
    // read
    writeData(SRAM_ADDR_REG, &addr, 1);
    readData(SRAM_DATA_REG, data, MAX_TRANSFER_SIZE);
    // compare
    bool success = true;
    for (int i = 0; i < MAX_TRANSFER_SIZE; i++) {
        if (data[i] != i) {
            success = false;
            ESP_LOGD(TAG, "selfTest failed [%d/%d]", i, MAX_TRANSFER_SIZE);
            break;
        }
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
    devcfg.clock_speed_hz = 50000;
    devcfg.spics_io_num = -1;
    devcfg.flags = SPI_DEVICE_HALFDUPLEX;
    devcfg.dummy_bits = 0;
    devcfg.queue_size = 1;
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &device_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "run spi_bus_add_device (READ) failed (FATAL)");
        return;
    }

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

    if (selfTest() == false) {
        ESP_LOGE(TAG, "run self test failed (FATAL)");
        return;
    }

    // periodic read time
    uint8_t raw[] = { 0, 0, 0, 0, 0, 0, 0 };
    struct tm structured;
    while (true) {

        readData(TIME_REG, raw, 7);
        ESP_LOGI(TAG, "raw: %02X %02X %02X %02X %02X %02X %02X", raw[0], raw[1],
                raw[2], raw[3], raw[4], raw[5], raw[6]);
        decodeRawTime(raw, &structured);
        ESP_LOGI(TAG, "%02d:%02d:%02d, wday:%d, mday:%d, mon:%d, year:%d, ",
                structured.tm_hour, structured.tm_min, structured.tm_sec,
                structured.tm_wday, structured.tm_mday, structured.tm_mon,
                structured.tm_year);

        time_t timestamp = mktime(&structured);
        pubsub_publish_int(timestamp_topic, timestamp);

        vTaskDelay(DS3432_LOOK_INTERVAL_MS / portTICK_PERIOD_MS);
    };
}

