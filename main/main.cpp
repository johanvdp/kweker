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

    led.setup(GPIO_LED, true, TOPIC_ACTIVITY);
    lamp.setup(GPIO_LAMP, true, TOPIC_ACTUATOR_LAMP);
    exhaust.setup(GPIO_EXHAUST, true, TOPIC_ACTUATOR_EXHAUST);
    recirc.setup(GPIO_RECIRC, true, TOPIC_ACTUATOR_RECIRC);
    heater.setup(GPIO_HEATER, true, TOPIC_ACTUATOR_HEATER);

    // LOG: big queue not useful
    QueueHandle_t log_queue = xQueueCreate(10, sizeof(pubsub_message_t));
    if (log_queue == 0) {
        ESP_LOGE(TAG, "setup, failed to create log queue (FATAL)");
        return;
    }
    pubsub_add_subscription(log_queue, TOPIC_AM2301_STATUS);
    pubsub_add_subscription(log_queue, TOPIC_MEASURED_TEMPERATURE);
    pubsub_add_subscription(log_queue, TOPIC_MEASURED_HUMIDITY);
    pubsub_add_subscription(log_queue, TOPIC_AM2301_TIMESTAMP);

    am2301.setup(GPIO_AM2301, measured_temperature_topic, measured_humidity_topic,
            am2301_status_topic, am2301_timestamp_topic);

    ds3234.setup(time_topic, TOPIC_TIME);

    // universal mixed message type can be received only
    pubsub_message_t log_message;

    // explicit message type required when publishing
    pubsub_message_t activity_message;
    activity_message.topic = (char *)TOPIC_ACTIVITY;
    activity_message.type = PUBSUB_TYPE_INT;

    // start chain reaction
    ESP_LOGI(TAG, "AM2301 measure");
    activity_message.int_val = 1;
    pubsub_publish(activity_topic, &activity_message);
    am2301.measure();

    while (1) {

        BaseType_t result = xQueueReceive(log_queue, &log_message,
                5000 / portTICK_PERIOD_MS);
        if (result == pdFALSE) {
            // nothing, re-try
            ESP_LOGI(TAG, "AM2301 measure");
            activity_message.int_val = 1;
            pubsub_publish(activity_topic, &activity_message);
            am2301.measure();

        } else {
            // something
            if (strcmp(log_message.topic, TOPIC_MEASURED_TEMPERATURE) == 0) {
                ESP_LOGI(TAG, "AM2301 T:%.1fK, %.1fC", log_message.double_val,
                        log_message.double_val - 273.15);

            } else if (strcmp(log_message.topic, TOPIC_MEASURED_HUMIDITY) == 0) {
                ESP_LOGI(TAG, "AM2301 RH:%.1f%%", log_message.double_val);

            } else if (strcmp(log_message.topic, TOPIC_AM2301_TIMESTAMP) == 0) {
                ESP_LOGI(TAG, "AM2301 time:%lld", log_message.int_val);

            } else if (strcmp(log_message.topic, TOPIC_AM2301_STATUS) == 0) {
                int64_t status = log_message.int_val;
                if (status == AM2301::result_status_t::RESULT_OK) {

                    ESP_LOGI(TAG, "AM2301 OK");

                    activity_message.int_val = 1;
                    pubsub_publish(activity_topic, &activity_message);

                    // again
                    vTaskDelay(5000 / portTICK_PERIOD_MS);

                    ESP_LOGI(TAG, "AM2301 measure");
                    activity_message.int_val = 1;
                    pubsub_publish(activity_topic, &activity_message);
                    am2301.measure();

                } else if (status
                        == AM2301::result_status_t::RESULT_RECOVERABLE) {

                    ESP_LOGW(TAG, "AM2301 RECOVERABLE");

                    activity_message.int_val = 2;
                    pubsub_publish(activity_topic, &activity_message);

                    // hold-off
                    vTaskDelay(10000 / portTICK_PERIOD_MS);

                    ESP_LOGI(TAG, "AM2301 measure");
                    activity_message.int_val = 1;
                    pubsub_publish(activity_topic, &activity_message);
                    am2301.measure();

                } else if (status == AM2301::result_status_t::RESULT_FATAL) {

                    ESP_LOGE(TAG, "AM2301 FATAL");

                    activity_message.int_val = 10;
                    ESP_LOGI(TAG, "pubsub_publish 10");
                    pubsub_publish(activity_topic, &activity_message);

                    // give up
                    break;

                } else {
                    ESP_LOGE(TAG, "status:%lld", status);
                }
            }
        }
    }
}
}
