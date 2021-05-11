// The author disclaims copyright to this source code.

extern "C" {

#include <string.h>

#include "driver/spi_master.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "pubsub.h"
#include "pubsub_test.h"
#include "LED.h"
#include "DO.h"
#include "AM2301.h"
#include "DS3234.h"
#include "MHZ19B.h"

#include "model.h"
#include "hmi.h"
#include "bind.h"
#include "ctrl.h"
#include "NVS.h"

#define TAG "main"

// GPIO configuration see Kconfig.projbuild
#define GPIO_LED (gpio_num_t)CONFIG_GPIO_LED
#define GPIO_AM2301 (gpio_num_t)CONFIG_GPIO_AM2301
#define GPIO_LIGHT (gpio_num_t)CONFIG_GPIO_LIGHT
#define GPIO_EXHAUST (gpio_num_t)CONFIG_GPIO_EXHAUST
#define GPIO_RECIRC (gpio_num_t)CONFIG_GPIO_RECIRC
#define GPIO_HEATER (gpio_num_t)CONFIG_GPIO_HEATER

#define SPI_HOST_A (spi_host_device_t)CONFIG_SPI_HOST_A
#define GPIO_SPI_A_MISO (gpio_num_t)CONFIG_GPIO_SPI_A_MISO
#define GPIO_SPI_A_MOSI (gpio_num_t)CONFIG_GPIO_SPI_A_MOSI
#define GPIO_SPI_A_CLK (gpio_num_t)CONFIG_GPIO_SPI_A_CLK

#define SPI_HOST_DS3234 (spi_host_device_t)CONFIG_SPI_HOST_DS3234
#define GPIO_DS3234_CS (gpio_num_t)CONFIG_GPIO_DS3234_CS

#define UART_PORT_MHZ19B (uart_port_t)CONFIG_UART_PORT_MHZ19B
#define GPIO_MHZ19B_TXD (gpio_num_t)CONFIG_GPIO_MHZ19B_TXD
#define GPIO_MHZ19B_RXD (gpio_num_t)CONFIG_GPIO_MHZ19B_RXD

#define SPI_HOST_MCP23S17 (spi_host_device_t)CONFIG_HOST_MCP23S17
#define GPIO_MCP23S17_CS (gpio_num_t)CONFIG_GPIO_MCP23S17_CS
#define GPIO_MCP23S17_RST (gpio_num_t)CONFIG_GPIO_MCP23S17_RST

#define AM2301_MEASUREMENT_PERIOD_MS 60000
#define MHZ19B_MEASUREMENT_PERIOD_MS 120000
#define NVS_HOLD_OFF_MS (60 * 1000)
/**
 * Limit for non-DMA SPI transfers.
 * Can not use DMA because need HALF DUPLEX transfers to avoid data corruption.
 */
#define MAX_TRANSFER_SIZE 64

LED led;
AM2301 am2301;
DS3234 ds3234;
DO light;
DO exhaust;
DO recirc;
DO heater;
NVS nvs;
MHZ19B mhz19b;

void nvs_setup()
{
    const char *nvs_settings[] { //
    MODEL_BEGIN_OF_DAY, //
            MODEL_BEGIN_OF_NIGHT, //
            MODEL_CO2_SV_DAY, //
            MODEL_CO2_SV_NIGHT, //
            MODEL_HUM_SV_DAY, //
            MODEL_HUM_SV_NIGHT, //
            MODEL_TEMP_SV_DAY, //
            MODEL_TEMP_SV_NIGHT, //
            MODEL_EXHAUST_SV, //
            MODEL_HEATER_SV, //
            MODEL_LIGHT_SV, //
            MODEL_RECIRC_SV //
    };

    nvs.setup("settings", nvs_settings, sizeof(nvs_settings) / sizeof(nvs_settings[0]), NVS_HOLD_OFF_MS);
}

void spi_setup() {
    // configure spi bus
    spi_bus_config_t buscfg;
    memset(&buscfg, 0, sizeof(spi_bus_config_t));
    buscfg.flags = SPICOMMON_BUSFLAG_IOMUX_PINS;
    buscfg.miso_io_num = GPIO_SPI_A_MISO;
    buscfg.mosi_io_num = GPIO_SPI_A_MOSI;
    buscfg.sclk_io_num = GPIO_SPI_A_CLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = MAX_TRANSFER_SIZE;
    esp_err_t ret = spi_bus_initialize(SPI_HOST_A, &buscfg, 0);
    ESP_ERROR_CHECK(ret);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "run, spi_bus_initialize failed (FATAL)");
        return;
    }
}

void app_main()
{
    ESP_LOGI(TAG, "app_main");

    BaseType_t ret = gpio_install_isr_service(
    ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "gpio_install_isr_service failed:%d (FATAL)", ret);
        return;
    }

    hmi_initialize();
    pubsub_initialize();

    // pubsub self test
    bool succes = pubsub_test();
    if (succes) {
        ESP_LOGI(TAG, "pubsub_test succes");
    } else {
        ESP_LOGE(TAG, "pubsub_test failed (FATAL)");
        return;
    }

    model_initialize();
    bind_initialize();
    ctrl_initialize();

    spi_setup();

    led.setup(GPIO_LED, true, MODEL_ACTIVITY);

// disabled needs to be connected through io expander
//    light.setup(GPIO_LIGHT, true, MODEL_LIGHT);
//    exhaust.setup(GPIO_EXHAUST, true, MODEL_EXHAUST);
//    recirc.setup(GPIO_RECIRC, true, MODEL_RECIRC);
//    heater.setup(GPIO_HEATER, true, MODEL_HEATER);

    nvs_setup();

    // LOG: big queue not useful
    QueueHandle_t log_queue = xQueueCreate(10, sizeof(pubsub_message_t));
    if (log_queue == 0) {
        ESP_LOGE(TAG, "failed to create log queue (FATAL)");
        return;
    }
    pubsub_add_subscription(log_queue, MODEL_AM2301_STATUS, false);

    am2301.setup(GPIO_AM2301, MODEL_TEMP_PV, MODEL_HUM_PV, MODEL_AM2301_STATUS, MODEL_AM2301_TIMESTAMP, AM2301_MEASUREMENT_PERIOD_MS);

    ds3234.setup(SPI_HOST_DS3234, GPIO_DS3234_CS, MODEL_CURRENT_TIME);

    mhz19b.setup(UART_PORT_MHZ19B, GPIO_MHZ19B_RXD, GPIO_MHZ19B_RXD, MODEL_CO2_PV, MHZ19B_MEASUREMENT_PERIOD_MS);

    // universal mixed message type can be received only
    pubsub_message_t log_message;

    while (1) {

        if (xQueueReceive(log_queue, &log_message, portMAX_DELAY)) {
            // something
            if (strcmp(log_message.topic, MODEL_AM2301_STATUS) == 0) {
                int64_t status = log_message.int_val;
                if (status == AM2301::result_status_t::RESULT_OK) {

                    ESP_LOGD(TAG, "AM2301 OK");

                    // blink 1x
                    pubsub_publish_int(MODEL_ACTIVITY, 1);

                } else if (status == AM2301::result_status_t::RESULT_RECOVERABLE) {

                    ESP_LOGW(TAG, "AM2301 RECOVERABLE");

                    // blink 2x
                    pubsub_publish_int(MODEL_ACTIVITY, 2);

                } else if (status == AM2301::result_status_t::RESULT_FATAL) {

                    ESP_LOGE(TAG, "AM2301 FATAL");
                    // give up
                    break;

                } else {
                    // unknown
                    ESP_LOGE(TAG, "AM2301 %lld", status);
                }
            }
        }
    }
}

}
