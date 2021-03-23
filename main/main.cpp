// The author disclaims copyright to this source code.
extern "C" {

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "string.h"
#include "hmi.h"
#include "LED.h"
#include "DO.h"
#include "AM2301.h"
#include "DS3234.h"
#include "pubsub.h"
#include "pubsub_test.h"
#include "model.h"
#include "bind.h"

#define TAG "main"

// GPIO configuration see Kconfig.projbuild
#define GPIO_LED (gpio_num_t)CONFIG_GPIO_LED
#define GPIO_AM2301 (gpio_num_t)CONFIG_GPIO_AM2301
#define GPIO_LAMP (gpio_num_t)CONFIG_GPIO_LAMP
#define GPIO_EXHAUST (gpio_num_t)CONFIG_GPIO_EXHAUST
#define GPIO_RECIRC (gpio_num_t)CONFIG_GPIO_RECIRC
#define GPIO_HEATER (gpio_num_t)CONFIG_GPIO_HEATER

#define MEASUREMENT_PERIOD_MS 5000

LED led;
AM2301 am2301;
DS3234 ds3234;
DO lamp;
DO exhaust;
DO recirc;
DO heater;

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

    led.setup(GPIO_LED, true, MODEL_ACTIVITY);
    lamp.setup(GPIO_LAMP, true, MODEL_LAMP);
    exhaust.setup(GPIO_EXHAUST, true, MODEL_EXHAUST);
    recirc.setup(GPIO_RECIRC, true, MODEL_RECIRC);
    heater.setup(GPIO_HEATER, true, MODEL_HEATER);

    // LOG: big queue not useful
    QueueHandle_t log_queue = xQueueCreate(10, sizeof(pubsub_message_t));
    if (log_queue == 0) {
        ESP_LOGE(TAG, "setup, failed to create log queue (FATAL)");
        return;
    }
    pubsub_add_subscription(log_queue, MODEL_AM2301_STATUS, false);

    am2301.setup(GPIO_AM2301, model_temp_pv, model_hum_pv, model_am2301_status,
            model_am2301_timestamp, MEASUREMENT_PERIOD_MS);

    ds3234.setup(model_time, MODEL_TIME);

    // universal mixed message type can be received only
    pubsub_message_t log_message;

    // explicit message type required when publishing
    pubsub_message_t activity_message;
    activity_message.topic = (char*) MODEL_ACTIVITY;
    activity_message.type = PUBSUB_TYPE_INT;

    while (1) {

        if (xQueueReceive(log_queue, &log_message, portMAX_DELAY)) {
            // something
            if (strcmp(log_message.topic, MODEL_AM2301_STATUS) == 0) {
                int64_t status = log_message.int_val;
                if (status == AM2301::result_status_t::RESULT_OK) {

                    ESP_LOGI(TAG, "AM2301 OK");

                    // blink 1x
                    activity_message.int_val = 1;
                    pubsub_publish(model_activity, &activity_message);

                } else if (status
                        == AM2301::result_status_t::RESULT_RECOVERABLE) {

                    ESP_LOGW(TAG, "AM2301 RECOVERABLE");

                    // blink 2x
                    activity_message.int_val = 2;
                    pubsub_publish(model_activity, &activity_message);

                } else if (status == AM2301::result_status_t::RESULT_FATAL) {

                    ESP_LOGE(TAG, "AM2301 FATAL");
                    // give up
                    break;

                } else {
                    // unknown
                    ESP_LOGE(TAG, "status:%lld", status);
                }
            }
        }
    }
}

}
