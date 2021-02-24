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
#include "pubsub.h"
#include "pubsub_test.h"

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
DO lamp;
DO exhaust;
DO recirc;
DO heater;

static const char *TOPIC_LED_ACTIVITY = "led.activity";
static const char *TOPIC_AM2301_TEMPERATURE = "am2301.temperature";
static const char *TOPIC_AM2301_HUMIDITY = "am2301.humidity";
static const char *TOPIC_AM2301_STATUS = "am2301.status";
static const char *TOPIC_AM2301_TIMESTAMP = "am2301.timestamp";
static const char *TOPIC_DO_LAMP = "do.lamp";
static const char *TOPIC_DO_EXHAUST = "do.exhaust";
static const char *TOPIC_DO_RECIRC = "do.recirc";
static const char *TOPIC_DO_HEATER = "do.heater";

void app_main()
{
    ESP_LOGI(TAG, "app_main");

    BaseType_t ret = gpio_install_isr_service(
    ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "gpio_install_isr_service failed: %d (FATAL)", ret);
        return;
    }

    hmi_initialize();
    pubsub_initialize();
    bool succes = pubsub_test();
    if (succes) {
        ESP_LOGI(TAG, "pubsub_test succes");
    } else {
        ESP_LOGE(TAG, "pubsub_test failed (FATAL)");
        return;
    }

    pubsub_topic_t led_activity_topic = pubsub_register_topic(
            TOPIC_LED_ACTIVITY);
    pubsub_topic_t am2301_temperature_topic = pubsub_register_topic(
            TOPIC_AM2301_TEMPERATURE);
    pubsub_topic_t am2301_humidity_topic = pubsub_register_topic(
            TOPIC_AM2301_HUMIDITY);
    pubsub_topic_t am2301_status_topic = pubsub_register_topic(
            TOPIC_AM2301_STATUS);
    pubsub_topic_t am2301_timestamp_topic = pubsub_register_topic(
            TOPIC_AM2301_TIMESTAMP);
    pubsub_topic_t do_lamp_topic = pubsub_register_topic(TOPIC_DO_LAMP);
    pubsub_topic_t do_exhaust_topic = pubsub_register_topic(TOPIC_DO_EXHAUST);
    pubsub_topic_t do_recirc_topic = pubsub_register_topic(TOPIC_DO_RECIRC);
    pubsub_topic_t do_heater_topic = pubsub_register_topic(TOPIC_DO_HEATER);

    led.setup(GPIO_LED, true, TOPIC_LED_ACTIVITY);
    lamp.setup(GPIO_LAMP, true, TOPIC_DO_LAMP);
    exhaust.setup(GPIO_EXHAUST, true, TOPIC_DO_EXHAUST);
    recirc.setup(GPIO_RECIRC, true, TOPIC_DO_RECIRC);
    heater.setup(GPIO_HEATER, true, TOPIC_DO_HEATER);

    // LOG: big queue not useful
    QueueHandle_t log_queue = xQueueCreate(10, sizeof(pubsub_message_t));
    if (log_queue == 0) {
        ESP_LOGE(TAG, "setup, failed to create log queue (FATAL)");
        return;
    }
    pubsub_add_subscription(log_queue, TOPIC_AM2301_STATUS);
    pubsub_add_subscription(log_queue, TOPIC_AM2301_TEMPERATURE);
    pubsub_add_subscription(log_queue, TOPIC_AM2301_HUMIDITY);
    pubsub_add_subscription(log_queue, TOPIC_AM2301_TIMESTAMP);

    am2301.setup(GPIO_AM2301, am2301_temperature_topic, am2301_humidity_topic,
            am2301_status_topic, am2301_timestamp_topic);

    pubsub_message_t log_message;
    pubsub_message_t led_activity_message;

    // start chain reaction
    led_activity_message.int_val = 1;
    pubsub_publish(led_activity_topic, &led_activity_message);
    am2301.measure();

    while (1) {

        BaseType_t result = xQueueReceive(log_queue, &log_message,
                5000 / portTICK_PERIOD_MS);
        if (result == pdFALSE) {
            // nothing, re-try
            ESP_LOGI(TAG, "AM2301 measure");
            led_activity_message.int_val = 1;
            pubsub_publish(led_activity_topic, &led_activity_message);
            am2301.measure();

        } else {
            // something
            if (strcmp(log_message.topic, TOPIC_AM2301_TEMPERATURE) == 0) {
                ESP_LOGI(TAG, "AM2301 T: %.1fK, %.1fC", log_message.double_val,
                        log_message.double_val - 273.15);

            } else if (strcmp(log_message.topic, TOPIC_AM2301_HUMIDITY) == 0) {
                ESP_LOGI(TAG, "AM2301 RH: %.1f%%", log_message.double_val);

            } else if (strcmp(log_message.topic, TOPIC_AM2301_TIMESTAMP) == 0) {
                ESP_LOGI(TAG, "AM2301 time: %lld", log_message.int_val);

            } else if (strcmp(log_message.topic, TOPIC_AM2301_STATUS) == 0) {
                int64_t status = log_message.int_val;
                if (status == AM2301::result_status_t::RESULT_OK) {

                    ESP_LOGI(TAG, "AM2301 OK");

                    led_activity_message.int_val = 1;
                    pubsub_publish(led_activity_topic, &led_activity_message);

                    // again
                    vTaskDelay(5000 / portTICK_PERIOD_MS);

                    ESP_LOGI(TAG, "AM2301 measure");
                    led_activity_message.int_val = 1;
                    pubsub_publish(led_activity_topic, &led_activity_message);
                    am2301.measure();

                } else if (status
                        == AM2301::result_status_t::RESULT_RECOVERABLE) {

                    ESP_LOGW(TAG, "AM2301 RECOVERABLE");

                    led_activity_message.int_val = 2;
                    pubsub_publish(led_activity_topic, &led_activity_message);

                    // hold-off
                    vTaskDelay(10000 / portTICK_PERIOD_MS);

                    ESP_LOGI(TAG, "AM2301 measure");
                    led_activity_message.int_val = 1;
                    pubsub_publish(led_activity_topic, &led_activity_message);
                    am2301.measure();

                } else if (status == AM2301::result_status_t::RESULT_FATAL) {

                    ESP_LOGE(TAG, "AM2301 FATAL");

                    led_activity_message.int_val = 10;
                    ESP_LOGI(TAG, "pubsub_publish 10");
                    pubsub_publish(led_activity_topic, &led_activity_message);

                    // give up
                    break;

                } else {
                    ESP_LOGE(TAG, "status: %lld", status);
                }
            }
        }
    }
}
}
