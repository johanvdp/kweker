// The author disclaims copyright to this source code.

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pubsub.h"

#include "model.h"
#include "hmi_settings.h"
#include "bind_settings.h"
#include "bind.h"

static const char *TAG = "bind_settings";

/** current time setting */
static QueueHandle_t bind_current_time;
/** begin of day setting */
static QueueHandle_t bind_begin_of_day;
/** begin of night setting */
static QueueHandle_t bind_begin_of_night;
/** temperature during day setting */
static QueueHandle_t bind_temp_day;
/** temperature during night setting */
static QueueHandle_t bind_temp_night;
/** humidity during day setting */
static QueueHandle_t bind_hum_day;
/** humidity during night setting */
static QueueHandle_t bind_hum_night;
/** co2 during day setting */
static QueueHandle_t bind_co2_day;
/** co2 during night setting */
static QueueHandle_t bind_co2_night;

void bind_settings_task()
{
    pubsub_message_t message;
    if (xQueueReceive(bind_current_time, &message, 0)) {
        hmi_settings_set_current_time(message.int_val);
    }
    if (xQueueReceive(bind_begin_of_day, &message, 0)) {
        hmi_settings_set_begin_of_day(message.int_val);
    }
    if (xQueueReceive(bind_begin_of_night, &message, 0)) {
        hmi_settings_set_begin_of_night(message.int_val);
    }
    if (xQueueReceive(bind_temp_day, &message, 0)) {
        hmi_settings_set_temp_day(message.double_val);
    }
    if (xQueueReceive(bind_temp_night, &message, 0)) {
        hmi_settings_set_temp_night(message.double_val);
    }
    if (xQueueReceive(bind_hum_day, &message, 0)) {
        hmi_settings_set_hum_day(message.double_val);
    }
    if (xQueueReceive(bind_hum_night, &message, 0)) {
        hmi_settings_set_hum_night(message.double_val);
    }
    if (xQueueReceive(bind_co2_day, &message, 0)) {
        hmi_settings_set_co2_day(message.double_val);
    }
    if (xQueueReceive(bind_co2_night, &message, 0)) {
        hmi_settings_set_co2_night(message.double_val);
    }
}

static void bind_settings_subscribe()
{
    bind_current_time = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_current_time, MODEL_CURRENT_TIME, true);

    bind_begin_of_day = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_begin_of_day, MODEL_BEGIN_OF_DAY, true);

    bind_begin_of_night = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_begin_of_night, MODEL_BEGIN_OF_NIGHT, true);

    bind_temp_day = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_temp_day, MODEL_TEMP_SV_DAY, true);

    bind_temp_night = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_temp_night, MODEL_TEMP_SV_NIGHT, true);

    bind_hum_day = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_hum_day, MODEL_HUM_SV_DAY, true);

    bind_hum_night = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_hum_night, MODEL_HUM_SV_NIGHT, true);

    bind_co2_day = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_co2_day, MODEL_CO2_SV_DAY, true);

    bind_co2_night = xQueueCreate(BIND_QUEUE_DEPTH, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_co2_night, MODEL_CO2_SV_NIGHT, true);
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

static void bind_settings_temp_day_callback(double value) {
    pubsub_publish_double(model_temp_sv_day, value);
}

static void bind_settings_temp_night_callback(double value) {
    pubsub_publish_double(model_temp_sv_night, value);
}

static void bind_settings_hum_day_callback(double value) {
    pubsub_publish_double(model_hum_sv_day, value);
}

static void bind_settings_hum_night_callback(double value) {
    pubsub_publish_double(model_hum_sv_night, value);
}

static void bind_settings_co2_day_callback(double value) {
    pubsub_publish_double(model_co2_sv_day, value);
}

static void bind_settings_co2_night_callback(double value) {
    pubsub_publish_double(model_co2_sv_night, value);
}

void bind_settings_initialize()
{
    bind_settings_subscribe();

    hmi_settings_set_current_time_callback(&bind_settings_current_time_callback);
    hmi_settings_set_begin_of_day_callback(&bind_settings_begin_of_day_callback);
    hmi_settings_set_begin_of_night_callback(&bind_settings_begin_of_night_callback);
    hmi_settings_set_temp_day_callback(&bind_settings_temp_day_callback);
    hmi_settings_set_temp_night_callback(&bind_settings_temp_night_callback);
    hmi_settings_set_hum_day_callback(&bind_settings_hum_day_callback);
    hmi_settings_set_hum_night_callback(&bind_settings_hum_night_callback);
    hmi_settings_set_co2_day_callback(&bind_settings_co2_day_callback);
    hmi_settings_set_co2_night_callback(&bind_settings_co2_night_callback);
}
