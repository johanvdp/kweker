// The author disclaims copyright to this source code.

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "pubsub.h"

#include "model.h"
#include "hmi.h"
#include "bind.h"
#include "bind_control.h"
#include "bind_settings.h"

static const char *TAG = "bind";

/** toolbar exhaust indicator */
static QueueHandle_t exhaust;
/** toolbar heater indicator */
static QueueHandle_t heater;
/** toolbar light indicator */
static QueueHandle_t light;
/** toolbar recirculation indicator */
static QueueHandle_t recirc;
/** toolbar circadian indicator */
static QueueHandle_t circadian;
/** toolbar control mode indicator */
static QueueHandle_t control_mode;
/** toolbar current time indicator */
static QueueHandle_t current_time;

static void bind_task(void *pvParameter)
{
    pubsub_message_t message;
    while (true) {
        if (xQueueReceive(exhaust, &message, 0)) {
            hmi_set_exhaust(message.boolean_val);
        }
        if (xQueueReceive(heater, &message, 0)) {
            hmi_set_heater(message.boolean_val);
        }
        if (xQueueReceive(light, &message, 0)) {
            hmi_set_light(message.boolean_val);
        }
        if (xQueueReceive(recirc, &message, 0)) {
            hmi_set_recirc(message.boolean_val);
        }
        if (xQueueReceive(circadian, &message, 0)) {
            model_circadian_t circadian = (model_circadian_t) message.int_val;
            hmi_set_circadian(circadian);
        }
        if (xQueueReceive(control_mode, &message, 0)) {
            model_control_mode_t mode = (model_control_mode_t) message.int_val;
            if (mode == MODEL_CONTROL_MODE_OFF) {
                hmi_set_control_mode(HMI_CONTROL_MODE_OFF);
            } else if (mode == MODEL_CONTROL_MODE_MANUAL) {
                hmi_set_control_mode(HMI_CONTROL_MODE_MANUAL);
            } else if (mode == MODEL_CONTROL_MODE_AUTO) {
                hmi_set_control_mode(HMI_CONTROL_MODE_AUTO);
            }
        }
        if (xQueueReceive(current_time, &message, 0)) {
            hmi_set_current_time(message.int_val);
        }

        bind_control_task();
        bind_settings_task();

        vTaskDelay(1);
    };
}

static void bind_subscribe()
{
    exhaust = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(exhaust, MODEL_EXHAUST, true);

    heater = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(heater, MODEL_HEATER, true);

    light = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(light, MODEL_LIGHT, true);

    recirc = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(recirc, MODEL_RECIRC, true);

    circadian = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(circadian, MODEL_CIRCADIAN, true);

    control_mode = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(control_mode, MODEL_CONTROL_MODE, true);

    current_time = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(current_time, MODEL_CURRENT_TIME, true);
}

void bind_initialize()
{
    ESP_LOGD(TAG, "bind_initialize");

    bind_subscribe();

    bind_control_initialize();
    bind_settings_initialize();

    BaseType_t ret = xTaskCreate(&bind_task, TAG, 2048, NULL, (tskIDLE_PRIORITY + 1), NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "failed to create task (FATAL)");
    }
}
