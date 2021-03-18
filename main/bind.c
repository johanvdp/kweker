// The author disclaims copyright to this source code.
#include "bind.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "model.h"
#include "pubsub.h"
#include "hmi.h"
#include "bind_control.h"
#include "bind_settings.h"

static const char* TAG = "hmi_bind";

/** toolbar exhaust indicator */
static QueueHandle_t actuator_exhaust_queue;
/** toolbar heater indicator */
static QueueHandle_t actuator_heater_queue;
/** toolbar lamp indicator */
static QueueHandle_t actuator_lamp_queue;
/** toolbar recirculation indicator */
static QueueHandle_t actuator_recirc_queue;
/** toolbar circadian indicator */
static QueueHandle_t circadian_queue;
/** toolbar control mode indicator */
static QueueHandle_t control_mode_queue;
/** toolbar time indicator */
static QueueHandle_t time_queue;

static void bind_task(void *pvParameter)
{
    pubsub_message_t message;
    while (true) {
        if (xQueueReceive(actuator_exhaust_queue, &message, 0)) {

        }
        if (xQueueReceive(actuator_heater_queue, &message, 0)) {

        }
        if (xQueueReceive(actuator_lamp_queue, &message, 0)) {

        }
        if (xQueueReceive(actuator_recirc_queue, &message, 0)) {

        }
        if (xQueueReceive(circadian_queue, &message, 0)) {

        }
        if (xQueueReceive(control_mode_queue, &message, 0)) {

        }
        if (xQueueReceive(time_queue, &message, 0)) {
            ESP_LOGD(TAG, "receive time:%lld", message.int_val);

            hmi_set_clock(message.int_val);
        }
        vTaskDelay(1);
    };
}

void bind_initialize()
{
    actuator_exhaust_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(actuator_exhaust_queue, TOPIC_ACTUATOR_EXHAUST);

    actuator_heater_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(actuator_heater_queue, TOPIC_ACTUATOR_HEATER);

    actuator_lamp_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(actuator_lamp_queue, TOPIC_ACTUATOR_LAMP);

    actuator_recirc_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(actuator_recirc_queue, TOPIC_ACTUATOR_RECIRC);

    circadian_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(circadian_queue, TOPIC_CIRCADIAN);

    control_mode_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(control_mode_queue, TOPIC_CONTROL_MODE);

    time_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(time_queue, TOPIC_TIME);

    BaseType_t ret = xTaskCreate(&bind_task, TAG, 2048, NULL,
            (tskIDLE_PRIORITY + 1), NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "failed to create task (FATAL)");
    }

    bind_control_initialize();
    bind_settings_initialize();
}
