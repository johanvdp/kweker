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
#include "ctrl.h"

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
        pubsub_publish_int(MODEL_CIRCADIAN, day ? MODEL_CIRCADIAN_DAY : MODEL_CIRCADIAN_NIGHT);
    }
}

void ctrl_circadian_task()
{
    pubsub_message_t message;
    bool change = false;
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
        begin_of_night_minutes = brokentime.tm_hour * 60 + brokentime.tm_min;
        change = true;
    }

    if (change) {
        // minute:    0...........1439
        // day night: nnnnnDdddddddNnn
        // night day: ddNnnnnnnnDddddd
        uint16_t day;
        if (begin_of_day_minutes < begin_of_night_minutes) {
            day = (time_minutes >= begin_of_day_minutes) && (time_minutes < begin_of_night_minutes);
        } else {
            day = (time_minutes < begin_of_night_minutes) || (time_minutes >= begin_of_day_minutes);
        }
        ctrl_circadian_set_day(day);
    }
}

static void ctrl_circadian_subscribe()
{
    time_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(time_queue, MODEL_CURRENT_TIME, true);
    begin_of_day_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(begin_of_day_queue, MODEL_BEGIN_OF_DAY, true);
    begin_of_night_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(begin_of_night_queue, MODEL_BEGIN_OF_NIGHT, true);
}

void ctrl_circadian_initialize()
{
    ESP_LOGD(TAG, "ctrl_circadian_initialize");

    ctrl_circadian_subscribe();
}

