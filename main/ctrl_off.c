// The author disclaims copyright to this source code.

#include <stdbool.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pubsub.h"

#include "model.h"

#include "ctrl_off.h"
#include "ctrl.h"

static const char *TAG = "ctrl_off";

static QueueHandle_t control_mode_queue;

void ctrl_off_task()
{
    pubsub_message_t message;

    if (xQueueReceive(control_mode_queue, &message, 0)) {
        model_control_mode_t control_mode = message.int_val;
        if (control_mode == MODEL_CONTROL_MODE_OFF) {
            pubsub_publish_bool(MODEL_LIGHT_SV, false);
            pubsub_publish_bool(MODEL_LIGHT, false);
            pubsub_publish_bool(MODEL_EXHAUST_SV, false);
            pubsub_publish_bool(MODEL_EXHAUST, false);
            pubsub_publish_bool(MODEL_RECIRC_SV, false);
            pubsub_publish_bool(MODEL_RECIRC, false);
            pubsub_publish_bool(MODEL_HEATER_SV, false);
            pubsub_publish_bool(MODEL_HEATER, false);
        }
    }
}

static void ctrl_off_subscribe()
{
    control_mode_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(control_mode_queue, MODEL_CONTROL_MODE, true);
}

void ctrl_off_initialize()
{
    ESP_LOGD(TAG, "ctrl_off_initialize");

    ctrl_off_subscribe();
}

