// The author disclaims copyright to this source code.

#include <stdbool.h>
#include <time.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pubsub.h"

#include "model.h"

#include "ctrl_circadian.h"

static const char *TAG = "ctrl_circadian";

static QueueHandle_t time_queue;
static uint16_t time_minutes;
static QueueHandle_t begin_of_day_queue;
static uint16_t begin_of_day_minutes;
static QueueHandle_t begin_of_night_queue;
static uint16_t begin_of_night_minutes;
static bool day;

static void ctrl_circadian_set_day(bool value)
{
    if (day != value) {
        day = value;
        pubsub_publish_int(model_circadian,
                day ? MODEL_CIRCADIAN_DAY : MODEL_CIRCADIAN_NIGHT);
    }
}

static void ctrl_circadian_task(void *pvParameter)
{
    pubsub_message_t message;
    bool change;
    while (true) {
        change = false;
        if (xQueueReceive(time_queue, &message, 0)) {
            struct tm brokentime;
            time_t time = message.int_val;
            gmtime_r(&time, &brokentime);
            time_minutes = brokentime.tm_hour * 60 + brokentime.tm_min;
            change = true;
        }
        if (xQueueReceive(begin_of_day_queue, &message, 0)) {
            struct tm brokentime;
            time_t time = message.int_val;
            gmtime_r(&time, &brokentime);
            begin_of_day_minutes = brokentime.tm_hour * 60 + brokentime.tm_min;
            change = true;
        }
        if (xQueueReceive(begin_of_night_queue, &message, 0)) {
            struct tm brokentime;
            time_t time = message.int_val;
            gmtime_r(&time, &brokentime);
            begin_of_night_minutes = brokentime.tm_hour * 60
                    + brokentime.tm_min;
            change = true;
        }

        if (change) {
            // minute:    0...........1439
            // day night: nnnnnDdddddddNnn
            // night day: ddNnnnnnnnDddddd
            uint16_t day;
            if (begin_of_day_minutes < begin_of_night_minutes) {
                day = (time_minutes >= begin_of_day_minutes)
                        && (time_minutes < begin_of_night_minutes);
            } else {
                day = (time_minutes < begin_of_night_minutes)
                        || (time_minutes >= begin_of_day_minutes);
            }
            ctrl_circadian_set_day(day);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    };
}

static void ctrl_circadian_subscribe()
{
    time_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(time_queue, MODEL_CURRENT_TIME, true);
    begin_of_day_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(begin_of_day_queue, MODEL_BEGIN_OF_DAY, true);
    begin_of_night_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(begin_of_night_queue, MODEL_BEGIN_OF_NIGHT, true);
}

void ctrl_circadian_initialize()
{
    ctrl_circadian_subscribe();

    BaseType_t ret = xTaskCreate(&ctrl_circadian_task, TAG, 2048, NULL,
            (tskIDLE_PRIORITY + 1),
            NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "failed to create task (FATAL)");
    }
}

