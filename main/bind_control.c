// The author disclaims copyright to this source code.

#include <stdbool.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pubsub.h"

#include "model.h"
#include "hmi_control.h"
#include "bind_control.h"

static const char *TAG = "bind_control";

/** selection buttons control mode */
static QueueHandle_t control_mode;

/** Automatic control CO2 concentration high (boolean) */
static QueueHandle_t co2_hi;
/** Automatic control CO2 concentration low (boolean) */
static QueueHandle_t co2_lo;
/** Automatic control humidity high (boolean) */
static QueueHandle_t hum_hi;
/** Automatic control humidity low (boolean) */
static QueueHandle_t hum_lo;
/** Automatic control temperature high (boolean) */
static QueueHandle_t temp_hi;
/** Automatic control temperature low (boolean) */
static QueueHandle_t temp_lo;

/** manual control setpoint exhaust fan */
static QueueHandle_t exhaust_sv;
/** manual scontrol etpoint heater */
static QueueHandle_t heater_sv;
/** manual control setpoint light */
static QueueHandle_t light_sv;
/** manual control setpoint recirculation fan */
static QueueHandle_t recirc_sv;

/** automatic control setpoint co2 concentration */
static QueueHandle_t co2_sv;
/** automatic control setpoint humidity */
static QueueHandle_t hum_sv;
/** automatic control setpoint temperature */
static QueueHandle_t temp_sv;

/** measurement co2 concentration */
static QueueHandle_t co2_pv;
/** measurement humidity */
static QueueHandle_t hum_pv;
/** measurement temperature */
static QueueHandle_t temp_pv;

static void bind_control_task(void *pvParameter)
{
    pubsub_message_t message;
    while (true) {

        if (xQueueReceive(control_mode, &message, 0)) {
            model_control_mode_t mode = (model_control_mode_t) message.int_val;
            if (mode == MODEL_CONTROL_MODE_OFF) {
                hmi_control_set_control_mode(HMI_CONTROL_MODE_OFF);
            } else if (mode == MODEL_CONTROL_MODE_MANUAL) {
                hmi_control_set_control_mode(HMI_CONTROL_MODE_MANUAL);
            } else if (mode == MODEL_CONTROL_MODE_AUTO) {
                hmi_control_set_control_mode(HMI_CONTROL_MODE_AUTO);
            }
        }

        if (xQueueReceive(co2_sv, &message, 0)) {
            hmi_control_set_co2_sv(message.double_val);
        }
        if (xQueueReceive(hum_sv, &message, 0)) {
            hmi_control_set_hum_sv(message.double_val);
        }
        if (xQueueReceive(temp_sv, &message, 0)) {
            hmi_control_set_temp_sv(message.double_val);
        }

        if (xQueueReceive(exhaust_sv, &message, 0)) {
            hmi_control_set_exhaust_sv(message.boolean_val);
        }
        if (xQueueReceive(heater_sv, &message, 0)) {
            hmi_control_set_heater_sv(message.boolean_val);
        }
        if (xQueueReceive(light_sv, &message, 0)) {
            hmi_control_set_light_sv(message.boolean_val);
        }
        if (xQueueReceive(recirc_sv, &message, 0)) {
            hmi_control_set_recirc_sv(message.boolean_val);
        }

        if (xQueueReceive(co2_pv, &message, 0)) {
            hmi_control_set_co2_pv(message.double_val);
        }
        if (xQueueReceive(hum_pv, &message, 0)) {
            hmi_control_set_hum_pv(message.double_val);
        }
        if (xQueueReceive(temp_pv, &message, 0)) {
            hmi_control_set_temp_pv(message.double_val);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    };
}

static void bind_control_subscribe()
{
    control_mode = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(control_mode, MODEL_CONTROL_MODE, true);

    exhaust_sv = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(exhaust_sv, MODEL_EXHAUST_SV, true);

    heater_sv = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(heater_sv, MODEL_HEATER_SV, true);

    light_sv = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(light_sv, MODEL_LIGHT_SV, true);

    recirc_sv = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(recirc_sv, MODEL_RECIRC_SV, true);

    co2_sv = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(co2_sv, MODEL_CO2_SV, true);

    hum_sv = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(hum_sv, MODEL_HUM_SV, true);

    temp_sv = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(temp_sv, MODEL_TEMP_SV, true);

    co2_pv = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(co2_pv, MODEL_CO2_PV, true);

    hum_pv = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(hum_pv, MODEL_HUM_PV, true);

    temp_pv = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(temp_pv, MODEL_TEMP_PV, true);
}

static void bind_control_mode_callback(hmi_control_mode_t mode)
{
    pubsub_publish_int(model_control_mode, mode);
}

static void bind_control_light_sv_callback(bool active)
{
    pubsub_publish_bool(model_light_sv, active);
}

static void bind_control_exhaust_sv_callback(bool active)
{
    pubsub_publish_bool(model_exhaust_sv, active);
}

static void bind_control_recirc_sv_callback(bool active)
{
    pubsub_publish_bool(model_recirc_sv, active);
}

static void bind_control_heater_sv_callback(bool active)
{
    pubsub_publish_bool(model_heater_sv, active);
}

void bind_control_initialize()
{
    bind_control_subscribe();

    hmi_control_set_control_mode_callback(&bind_control_mode_callback);
    hmi_control_set_light_sv_callback(&bind_control_light_sv_callback);
    hmi_control_set_exhaust_sv_callback(&bind_control_exhaust_sv_callback);
    hmi_control_set_recirc_sv_callback(&bind_control_recirc_sv_callback);
    hmi_control_set_heater_sv_callback(&bind_control_heater_sv_callback);

    BaseType_t ret = xTaskCreate(&bind_control_task, TAG, 2048, NULL,
            (tskIDLE_PRIORITY + 1),
            NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "failed to create task (FATAL)");
    }
}

