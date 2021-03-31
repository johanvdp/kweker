// The author disclaims copyright to this source code.

#include <stdbool.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pubsub.h"

#include "model.h"

#include "ctrl_manual.h"
#include "ctrl.h"

static const char *TAG = "ctrl_manual";

static QueueHandle_t control_mode_queue;
static model_control_mode_t control_mode;

static QueueHandle_t light_queue;
static QueueHandle_t light_sv_queue;

static QueueHandle_t exhaust_queue;
static QueueHandle_t exhaust_sv_queue;

static QueueHandle_t recirc_queue;
static QueueHandle_t recirc_sv_queue;

static QueueHandle_t heater_queue;
static QueueHandle_t heater_sv_queue;

void ctrl_manual_task()
{
    pubsub_message_t message;

    if (xQueueReceive(control_mode_queue, &message, 0)) {
        control_mode = message.int_val;
    }
    if (xQueueReceive(light_sv_queue, &message, 0)) {
        if (control_mode == MODEL_CONTROL_MODE_MANUAL) {
            bool light_sv = message.boolean_val;
            pubsub_publish_bool(model_light, light_sv);
        }
    }
    if (xQueueReceive(exhaust_sv_queue, &message, 0)) {
        if (control_mode == MODEL_CONTROL_MODE_MANUAL) {
            bool exhaust_sv = message.boolean_val;
            pubsub_publish_bool(model_exhaust, exhaust_sv);
        }
    }
    if (xQueueReceive(recirc_sv_queue, &message, 0)) {
        if (control_mode == MODEL_CONTROL_MODE_MANUAL) {
            bool recirc_sv = message.boolean_val;
            pubsub_publish_bool(model_recirc, recirc_sv);
        }
    }
    if (xQueueReceive(heater_sv_queue, &message, 0)) {
        if (control_mode == MODEL_CONTROL_MODE_MANUAL) {
            bool heater_sv = message.boolean_val;
            pubsub_publish_bool(model_heater, heater_sv);
        }
    }
    if (xQueueReceive(light_queue, &message, 0)) {
        if (control_mode != MODEL_CONTROL_MODE_MANUAL) {
            bool light_on = message.boolean_val;
            pubsub_publish_bool(model_light_sv, light_on);
        }
    }
    if (xQueueReceive(exhaust_queue, &message, 0)) {
        if (control_mode != MODEL_CONTROL_MODE_MANUAL) {
            bool exhaust_on = message.boolean_val;
            pubsub_publish_bool(model_exhaust_sv, exhaust_on);
        }
    }
    if (xQueueReceive(recirc_queue, &message, 0)) {
        if (control_mode != MODEL_CONTROL_MODE_MANUAL) {
            bool recirc_on = message.boolean_val;
            pubsub_publish_bool(model_recirc_sv, recirc_on);
        }
    }
    if (xQueueReceive(heater_queue, &message, 0)) {
        if (control_mode != MODEL_CONTROL_MODE_MANUAL) {
            bool heater_on = message.boolean_val;
            pubsub_publish_bool(model_heater_sv, heater_on);
        }
    }
}

static void ctrl_manual_subscribe()
{
    control_mode_queue = xQueueCreate(CTRL_QUEUE_DEPTH,
            sizeof(pubsub_message_t));
    pubsub_add_subscription(control_mode_queue, MODEL_CONTROL_MODE, true);

    light_sv_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(light_sv_queue, MODEL_LIGHT_SV, true);

    exhaust_sv_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(exhaust_sv_queue, MODEL_EXHAUST_SV, true);

    recirc_sv_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(recirc_sv_queue, MODEL_RECIRC_SV, true);

    heater_sv_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(heater_sv_queue, MODEL_HEATER_SV, true);

    light_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(light_queue, MODEL_LIGHT, true);

    exhaust_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(exhaust_queue, MODEL_EXHAUST, true);

    recirc_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(recirc_queue, MODEL_RECIRC, true);

    heater_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(heater_queue, MODEL_HEATER, true);
}

void ctrl_manual_initialize()
{
    ctrl_manual_subscribe();
}

