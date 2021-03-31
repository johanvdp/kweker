// The author disclaims copyright to this source code.

#include <stdbool.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pubsub.h"

#include "model.h"

#include "ctrl_day_night.h"

static const char *TAG = "ctrl_day_night";

/** circadian */
static QueueHandle_t circadian_queue;
static model_circadian_t circadian;

/** automatic control setpoint day time co2 concentration */
static QueueHandle_t co2_sv_day_queue;
static double co2_sv_day;
/** automatic control setpoint night time co2 concentration */
static QueueHandle_t co2_sv_night_queue;
static double co2_sv_night;
static double co2_sv;

/** automatic control setpoint day time humidity */
static QueueHandle_t hum_sv_day_queue;
static double hum_sv_day;
/** automatic control setpoint night time humidity */
static QueueHandle_t hum_sv_night_queue;
static double hum_sv_night;
static double hum_sv;

/** automatic control setpoint day time temperature */
static QueueHandle_t temp_sv_day_queue;
static double temp_sv_day;
/** automatic control setpoint night time temperature */
static QueueHandle_t temp_sv_night_queue;
static double temp_sv_night;
static double temp_sv;

static void ctrl_day_night_set_co2_sv(double value) {
    if (value != co2_sv) {
        co2_sv = value;
        pubsub_publish_double(model_co2_sv, value);
    }
}

static void ctrl_day_night_set_hum_sv(double value) {
    if (value != hum_sv) {
        hum_sv = value;
        pubsub_publish_double(model_hum_sv, value);
    }
}

static void ctrl_day_night_set_temp_sv(double value) {
    if (value != temp_sv) {
        temp_sv = value;
        pubsub_publish_double(model_temp_sv, value);
    }
}

static void ctrl_day_night_set_circadian(model_circadian_t value) {
    if (value == MODEL_CIRCADIAN_DAY) {
        ctrl_day_night_set_co2_sv(co2_sv_day);
        ctrl_day_night_set_hum_sv(hum_sv_day);
        ctrl_day_night_set_temp_sv(temp_sv_day);
    } else {
        ctrl_day_night_set_co2_sv(co2_sv_night);
        ctrl_day_night_set_hum_sv(hum_sv_night);
        ctrl_day_night_set_temp_sv(temp_sv_night);
    }
}

static void ctrl_day_night_task(void *pvParameter)
{
    pubsub_message_t message;
    while (true) {
        if (xQueueReceive(circadian_queue, &message, 0)) {
            circadian = message.int_val;
            ctrl_day_night_set_circadian(circadian);
        }

        if (xQueueReceive(co2_sv_day_queue, &message, 0)) {
            co2_sv_day = message.double_val;
            if (circadian == MODEL_CIRCADIAN_DAY) {
                ctrl_day_night_set_co2_sv(co2_sv_day);
            }
        }
        if (xQueueReceive(co2_sv_night_queue, &message, 0)) {
            co2_sv_night = message.double_val;
            if (circadian == MODEL_CIRCADIAN_NIGHT) {
                ctrl_day_night_set_co2_sv(co2_sv_night);
            }
        }

        if (xQueueReceive(hum_sv_day_queue, &message, 0)) {
            hum_sv_day = message.double_val;
            if (circadian == MODEL_CIRCADIAN_DAY) {
                ctrl_day_night_set_hum_sv(hum_sv_day);
            }
        }
        if (xQueueReceive(hum_sv_night_queue, &message, 0)) {
            hum_sv_night = message.double_val;
            if (circadian == MODEL_CIRCADIAN_NIGHT) {
                ctrl_day_night_set_hum_sv(hum_sv_night);
            }
        }

        if (xQueueReceive(temp_sv_day_queue, &message, 0)) {
            temp_sv_day = message.double_val;
            if (circadian == MODEL_CIRCADIAN_DAY) {
                ctrl_day_night_set_temp_sv(temp_sv_day);
            }
        }
        if (xQueueReceive(temp_sv_night_queue, &message, 0)) {
            temp_sv_night = message.double_val;
            if (circadian == MODEL_CIRCADIAN_NIGHT) {
                ctrl_day_night_set_temp_sv(temp_sv_night);
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    };
}

static void ctrl_day_night_subscribe()
{
    circadian_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(circadian_queue, MODEL_CIRCADIAN, true);

    co2_sv_day_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(co2_sv_day_queue, MODEL_CO2_SV_DAY, true);
    co2_sv_night_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(co2_sv_night_queue, MODEL_CO2_SV_NIGHT, true);

    hum_sv_day_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(hum_sv_day_queue, MODEL_HUM_SV_DAY, true);
    hum_sv_night_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(hum_sv_night_queue, MODEL_HUM_SV_NIGHT, true);

    temp_sv_day_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(temp_sv_day_queue, MODEL_TEMP_SV_DAY, true);
    temp_sv_night_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(temp_sv_night_queue, MODEL_TEMP_SV_NIGHT, true);
}

void ctrl_day_night_initialize()
{
    ctrl_day_night_subscribe();

    BaseType_t ret = xTaskCreate(&ctrl_day_night_task, TAG, 2048, NULL,
            (tskIDLE_PRIORITY + 1),
            NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "failed to create task (FATAL)");
    }
}

