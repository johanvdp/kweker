// The author disclaims copyright to this source code.
#include "bind_settings.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "pubsub.h"
#include "model.h"
#include "hmi_settings.h"

static const char *TAG = "bind_settings";

/** current time setting */
static QueueHandle_t bind_current_time;
/** begin of day setting */
static QueueHandle_t bind_begin_of_day;
/** begin of night setting */
static QueueHandle_t bind_begin_of_night;

static void bind_settings_task(void *pvParameter)
{
    pubsub_message_t message;
    while (true) {
        if (xQueueReceive(bind_current_time, &message, 0)) {
            hmi_settings_set_current_time(message.int_val);
        }
        if (xQueueReceive(bind_begin_of_day, &message, 0)) {
            hmi_settings_set_begin_of_day(message.int_val);
        }
        if (xQueueReceive(bind_begin_of_night, &message, 0)) {
            hmi_settings_set_begin_of_night(message.int_val);
        }
        vTaskDelay(1);
    };
}

static void bind_settings_subscribe()
{
    bind_current_time = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_current_time, MODEL_CURRENT_TIME, true);

    bind_begin_of_day = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_begin_of_day, MODEL_BEGIN_OF_DAY, true);

    bind_begin_of_night = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_begin_of_night, MODEL_BEGIN_OF_NIGHT, true);
}

static void bind_settings_current_time_callback(time_t time) {
    pubsub_publish_int(model_current_time, time);
}

static void bind_settings_begin_of_day_callback(time_t time) {
    pubsub_publish_int(model_begin_of_day, time);
}

static void bind_settings_begin_of_night_callback(time_t time) {
    pubsub_publish_int(model_begin_of_night, time);
}

void bind_settings_initialize()
{
    bind_settings_subscribe();

    hmi_settings_set_current_time_callback(&bind_settings_current_time_callback);
    hmi_settings_set_begin_of_day_callback(&bind_settings_begin_of_day_callback);
    hmi_settings_set_begin_of_night_callback(&bind_settings_begin_of_night_callback);

    BaseType_t ret = xTaskCreate(&bind_settings_task, TAG, 2048, NULL,
            (tskIDLE_PRIORITY + 1), NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "failed to create task (FATAL)");
    }
}
