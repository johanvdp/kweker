// The author disclaims copyright to this source code.

#include <stdbool.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pubsub.h"

#include "model.h"

#include "ctrl_auto.h"
#include "ctrl.h"

static const char *TAG = "ctrl_auto";

/** circadian */
static QueueHandle_t circadian_queue;
static model_circadian_t circadian;

/** control mode */
static QueueHandle_t control_mode_queue;
static model_control_mode_t control_mode;

/** measurement co2 concentration */
static QueueHandle_t co2_pv_queue;
static double co2_pv;
/** automatic control setpoint co2 concentration */
static QueueHandle_t co2_sv_queue;
static double co2_sv;
static bool co2_lo;
static bool co2_hi;

/** measurement humidity */
static QueueHandle_t hum_pv_queue;
static double hum_pv;
/** automatic control setpoint humidity */
static QueueHandle_t hum_sv_queue;
static double hum_sv;
static bool hum_lo;
static bool hum_hi;

/** measurement temperature */
static QueueHandle_t temp_pv_queue;
static double temp_pv;
/** automatic control setpoint temperature */
static QueueHandle_t temp_sv_queue;
static double temp_sv;
static bool temp_lo;
static bool temp_hi;

static void ctrl_auto_set_co2_lo(bool value)
{
    if (value != co2_lo) {
        co2_lo = value;
        pubsub_publish_bool(model_co2_lo, value);
    }
}

static void ctrl_auto_set_co2_hi(bool value)
{
    if (value != co2_hi) {
        co2_hi = value;
        pubsub_publish_bool(model_co2_hi, value);
    }
}

static void ctrl_auto_set_hum_lo(bool value)
{
    if (value != hum_lo) {
        hum_lo = value;
        pubsub_publish_bool(model_hum_lo, value);
    }
}

static void ctrl_auto_set_hum_hi(bool value)
{
    if (value != hum_hi) {
        hum_hi = value;
        pubsub_publish_bool(model_hum_hi, value);
    }
}

static void ctrl_auto_set_temp_lo(bool value)
{
    if (value != temp_lo) {
        temp_lo = value;
        pubsub_publish_bool(model_temp_lo, value);
    }
}

static void ctrl_auto_set_temp_hi(bool value)
{
    if (value != temp_hi) {
        temp_hi = value;
        pubsub_publish_bool(model_temp_hi, value);
    }
}

static void ctrl_auto_light()
{
    bool light_on = (circadian == MODEL_CIRCADIAN_DAY);
    pubsub_publish_bool(model_light, light_on);
    pubsub_publish_bool(model_light_sv, light_on);
}

static void ctrl_auto_exhaust()
{
    bool exhaust_on = (temp_hi || hum_hi || co2_hi);
    pubsub_publish_bool(model_exhaust, exhaust_on);
    pubsub_publish_bool(model_exhaust_sv, exhaust_on);
}

static void ctrl_auto_recirculation()
{
    bool recirc_on = (circadian == MODEL_CIRCADIAN_DAY);
    pubsub_publish_bool(model_recirc, recirc_on);
    pubsub_publish_bool(model_recirc_sv, recirc_on);
}

static void ctrl_auto_heater()
{
    bool heater_on;
    // in order of importance
    if (temp_hi) {
        heater_on = false;
    } else if (temp_lo) {
        heater_on = true;
    } else if (hum_hi) {
        heater_on = true;
    } else {
        heater_on = false;
    }
    pubsub_publish_bool(model_heater, heater_on);
    pubsub_publish_bool(model_heater_sv, heater_on);
}

static void ctrl_auto_control()
{
    ctrl_auto_light();
    ctrl_auto_exhaust();
    ctrl_auto_recirculation();
    ctrl_auto_heater();
}

static void ctrl_auto_indicate()
{
    ctrl_auto_set_co2_lo(co2_pv < co2_sv);
    ctrl_auto_set_co2_hi(co2_pv > co2_sv);

    ctrl_auto_set_hum_lo(hum_pv < hum_sv);
    ctrl_auto_set_hum_hi(hum_pv > hum_sv);

    ctrl_auto_set_temp_lo(temp_pv < temp_sv);
    ctrl_auto_set_temp_hi(temp_pv > temp_sv);
}

void ctrl_auto_task()
{
    pubsub_message_t message;
    bool changed = false;

    if (xQueueReceive(circadian_queue, &message, 0)) {
        circadian = message.int_val;
        changed = true;
    }

    if (xQueueReceive(control_mode_queue, &message, 0)) {
        control_mode = message.int_val;
        changed = true;
    }

    if (xQueueReceive(co2_pv_queue, &message, 0)) {
        co2_pv = message.double_val;
        changed = true;
    }
    if (xQueueReceive(co2_sv_queue, &message, 0)) {
        co2_sv = message.double_val;
        changed = true;
    }

    if (xQueueReceive(hum_pv_queue, &message, 0)) {
        hum_pv = message.double_val;
        changed = true;
    }
    if (xQueueReceive(hum_sv_queue, &message, 0)) {
        hum_sv = message.double_val;
        changed = true;
    }

    if (xQueueReceive(temp_pv_queue, &message, 0)) {
        temp_pv = message.double_val;
        changed = true;
    }
    if (xQueueReceive(temp_sv_queue, &message, 0)) {
        temp_sv = message.double_val;
        changed = true;
    }

    ctrl_auto_indicate();

    if (control_mode == MODEL_CONTROL_MODE_AUTO && changed) {
        ctrl_auto_control();
    }
}

static void ctrl_auto_subscribe()
{
    circadian_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(circadian_queue, MODEL_CIRCADIAN, true);

    control_mode_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(control_mode_queue, MODEL_CONTROL_MODE, true);

    co2_pv_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(co2_pv_queue, MODEL_CO2_PV, true);
    co2_sv_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(co2_sv_queue, MODEL_CO2_SV, true);

    hum_pv_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(hum_pv_queue, MODEL_HUM_PV, true);
    hum_sv_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(hum_sv_queue, MODEL_HUM_SV, true);

    temp_pv_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(temp_pv_queue, MODEL_TEMP_PV, true);
    temp_sv_queue = xQueueCreate(CTRL_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(temp_sv_queue, MODEL_TEMP_SV, true);
}

void ctrl_auto_initialize()
{
    ESP_LOGD(TAG, "ctrl_auto_initialize");

    ctrl_auto_subscribe();
}

