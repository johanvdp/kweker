// The author disclaims copyright to this source code.
#include "bind_settings.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "pubsub.h"
#include "model.h"
#include "hmi_settings.h"

static const char* TAG = "bind_control";

/** set current time */
static void bind_settings_set_time(time_t time)
{

}

/** set begin of day time */
static void bind_settings_set_begin_of_day(time_t time)
{

}

/** set begin of night time */
static void bind_settings_set_begin_of_night(time_t time)
{

}

/** set day temperature */
static void bind_settings_set_day_temperature(double temperature)
{

}

/** set day humidity */
static void bind_settings_set_day_humidity(double humidity)
{

}

/** set day CO2 concentration */
static void bind_settings_set_day_co2(double co2)
{

}

/** set night temperature */
static void bind_settings_set_night_temperature(double temperature)
{

}

/** set night humidity */
static void bind_settings_set_night_humidity(double humidity)
{

}

/** set night CO2 concentration */
static void bind_settings_set_night_co2(double co2)
{

}

static void bind_settings_task(void *pvParameter)
{
    pubsub_message_t message;
    while (true) {
        vTaskDelay(1);
    };
}

void bind_settings_initialize() {

    BaseType_t ret = xTaskCreate(&bind_settings_task, TAG, 2048, NULL,
            (tskIDLE_PRIORITY + 1), NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "failed to create task (FATAL)");
    }
}
