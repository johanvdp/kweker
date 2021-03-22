// The author disclaims copyright to this source code.
#include "bind_control.h"

#include <freertos/FreeRTOS.h>
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "pubsub.h"
#include "model.h"
#include "hmi_control.h"

static const char *TAG = "bind_control";

/** selection buttons control mode */
static QueueHandle_t control_mode_queue;

/** Automatic control CO2 concentration high (boolean) */
static QueueHandle_t control_auto_co2_hi;
/** Automatic control CO2 concentration low (boolean) */
static QueueHandle_t control_auto_co2_lo;
/** Automatic control humidity high (boolean) */
static QueueHandle_t control_auto_humidity_hi;
/** Automatic control humidity low (boolean) */
static QueueHandle_t control_auto_humidity_lo;
/** Automatic control temperature high (boolean) */
static QueueHandle_t control_auto_temperature_hi;
/** Automatic control temperature low (boolean) */
static QueueHandle_t control_auto_temperature_lo;

/** manual control setpoint exhaust fan */
static QueueHandle_t manual_setpoint_exhaust_queue;
/** manual scontrol etpoint heater */
static QueueHandle_t manual_setpoint_heater_queue;
/** manual control setpoint lamp */
static QueueHandle_t manual_setpoint_lamp_queue;
/** manual control setpoint recirculation fan */
static QueueHandle_t manual_setpoint_recirc_queue;

/** automatic control setpoint day time co2 concentration */
static QueueHandle_t day_auto_setpoint_co2_queue;
/** automatic control setpoint day time humidity */
static QueueHandle_t day_auto_setpoint_humidity_queue;
/** automatic control setpoint day time temperature */
static QueueHandle_t day_auto_setpoint_temperature_queue;

/** measurement co2 concentration */
static QueueHandle_t measured_co2_queue;
/** measurement humidity */
static QueueHandle_t measured_humidity_queue;
/** measurement temperature */
static QueueHandle_t measured_temperature_queue;

/** automatic control setpoint night time co2 concentration */
static QueueHandle_t night_auto_setpoint_co2_queue;
/** automatic control setpoint night time humidity */
static QueueHandle_t night_auto_setpoint_humidity_queue;
/** automatic control setpoint night time temperature */
static QueueHandle_t night_auto_setpoint_temperature_queue;

static void hmi_control_set_temperature_hi(bool hi)
{
    hmi_control_set_hi(&hmi_control_temperature, hi);
}

static void hmi_control_set_temperature_lo(bool lo)
{
    hmi_control_set_lo(&hmi_control_temperature, lo);
}

static void hmi_control_set_humidity_hi(bool hi)
{
    hmi_control_set_hi(&hmi_control_humidity, hi);
}

static void hmi_control_set_humidity_lo(bool lo)
{
    hmi_control_set_lo(&hmi_control_humidity, lo);
}

static void hmi_control_set_co2_hi(bool hi)
{
    hmi_control_set_hi(&hmi_control_co2, hi);
}

static void hmi_control_set_co2_lo(bool lo)
{
    hmi_control_set_lo(&hmi_control_co2, lo);
}

static void bind_control_task(void *pvParameter)
{
    pubsub_message_t message;
    while (true) {
        if (xQueueReceive(control_mode_queue, &message, 0)) {
            hmi_control_set_control_mode(
                    (model_control_mode_t) message.int_val);
        }
        if (xQueueReceive(day_auto_setpoint_co2_queue, &message, 0)) {
            // if day
            hmi_control_set_sv(&hmi_control_co2, message.double_val);
        }
        if (xQueueReceive(day_auto_setpoint_humidity_queue, &message, 0)) {
            // if day
            hmi_control_set_sv(&hmi_control_humidity, message.double_val);
        }
        if (xQueueReceive(day_auto_setpoint_temperature_queue, &message, 0)) {
            // if day
            hmi_control_set_sv(&hmi_control_temperature, message.double_val);
        }
        if (xQueueReceive(manual_setpoint_exhaust_queue, &message, 0)) {

        }
        if (xQueueReceive(manual_setpoint_heater_queue, &message, 0)) {

        }
        if (xQueueReceive(manual_setpoint_lamp_queue, &message, 0)) {

        }
        if (xQueueReceive(manual_setpoint_recirc_queue, &message, 0)) {

        }
        if (xQueueReceive(measured_co2_queue, &message, 0)) {
            hmi_control_set_pv(&hmi_control_co2, message.double_val);
        }
        if (xQueueReceive(measured_humidity_queue, &message, 0)) {
            hmi_control_set_pv(&hmi_control_humidity, message.double_val);
        }
        if (xQueueReceive(measured_temperature_queue, &message, 0)) {
            // convert from Kelvin to degrees Celcius
            hmi_control_set_pv(&hmi_control_temperature,
                    message.double_val - 273.15);
        }
        if (xQueueReceive(night_auto_setpoint_co2_queue, &message, 0)) {
            // if night
            hmi_control_set_sv(&hmi_control_co2, message.double_val);
        }
        if (xQueueReceive(night_auto_setpoint_humidity_queue, &message, 0)) {
            // if night
            hmi_control_set_sv(&hmi_control_humidity, message.double_val);
        }
        if (xQueueReceive(night_auto_setpoint_temperature_queue, &message, 0)) {
            // if night
            hmi_control_set_sv(&hmi_control_temperature, message.double_val);
        }
        vTaskDelay(1);
    };
}

void bind_control_initialize()
{
    control_mode_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(control_mode_queue, TOPIC_CONTROL_MODE, true);

    manual_setpoint_exhaust_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(manual_setpoint_exhaust_queue,
            TOPIC_MANUAL_SETPOINT_EXHAUST, true);

    manual_setpoint_heater_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(manual_setpoint_heater_queue,
            TOPIC_MANUAL_SETPOINT_HEATER, true);

    manual_setpoint_lamp_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(manual_setpoint_lamp_queue,
            TOPIC_MANUAL_SETPOINT_LAMP, true);

    manual_setpoint_recirc_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(manual_setpoint_recirc_queue,
            TOPIC_MANUAL_SETPOINT_RECIRC, true);

    day_auto_setpoint_co2_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(day_auto_setpoint_co2_queue,
            TOPIC_DAY_AUTO_SETPOINT_CO2, true);

    day_auto_setpoint_humidity_queue = xQueueCreate(2,
            sizeof(pubsub_message_t));
    pubsub_add_subscription(day_auto_setpoint_humidity_queue,
            TOPIC_DAY_AUTO_SETPOINT_HUMIDITY, true);

    day_auto_setpoint_temperature_queue = xQueueCreate(2,
            sizeof(pubsub_message_t));
    pubsub_add_subscription(day_auto_setpoint_temperature_queue,
            TOPIC_DAY_AUTO_SETPOINT_TEMPERATURE, true);

    night_auto_setpoint_co2_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(night_auto_setpoint_co2_queue,
            TOPIC_NIGHT_AUTO_SETPOINT_CO2, true);

    night_auto_setpoint_humidity_queue = xQueueCreate(2,
            sizeof(pubsub_message_t));
    pubsub_add_subscription(night_auto_setpoint_humidity_queue,
            TOPIC_NIGHT_AUTO_SETPOINT_HUMIDITY, true);

    night_auto_setpoint_temperature_queue = xQueueCreate(2,
            sizeof(pubsub_message_t));
    pubsub_add_subscription(night_auto_setpoint_temperature_queue,
            TOPIC_NIGHT_AUTO_SETPOINT_TEMPERATURE, true);

    measured_co2_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(measured_co2_queue, TOPIC_MEASURED_CO2, true);

    measured_humidity_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(measured_humidity_queue, TOPIC_MEASURED_HUMIDITY, true);

    measured_temperature_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(measured_temperature_queue,
            TOPIC_MEASURED_TEMPERATURE, true);

    BaseType_t ret = xTaskCreate(&bind_control_task, TAG, 2048, NULL,
            (tskIDLE_PRIORITY + 1),
            NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "failed to create task (FATAL)");
    }
}

