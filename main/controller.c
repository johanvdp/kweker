// The author disclaims copyright to this source code.
#include "hmi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "hmi_control.h"
#include "hmi_settings.h"
#include "hmi_about.h"
#include "controller.h"
#include "model.h"
#include "pubsub.h"
#include "esp_log.h"

static const char *tag = "controller";

static QueueHandle_t actuator_exhaust_queue;
static QueueHandle_t actuator_heater_queue;
static QueueHandle_t actuator_lamp_queue;
static QueueHandle_t actuator_recirc_queue;
static QueueHandle_t circadian_queue;
static QueueHandle_t control_mode_queue;
static QueueHandle_t day_auto_setpoint_co2_queue;
static QueueHandle_t day_auto_setpoint_humidity_queue;
static QueueHandle_t day_auto_setpoint_temperature_queue;
static QueueHandle_t manual_setpoint_exhaust_queue;
static QueueHandle_t manual_setpoint_heater_queue;
static QueueHandle_t manual_setpoint_lamp_queue;
static QueueHandle_t manual_setpoint_recirc_queue;
static QueueHandle_t measured_co2_queue;
static QueueHandle_t measured_humidity_queue;
static QueueHandle_t measured_temperature_queue;
static QueueHandle_t night_auto_setpoint_co2_queue;
static QueueHandle_t night_auto_setpoint_humidity_queue;
static QueueHandle_t night_auto_setpoint_temperature_queue;

static void task(void *pvParameter);

void controller_initialize()
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

    day_auto_setpoint_co2_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(day_auto_setpoint_co2_queue, TOPIC_DAY_AUTO_SETPOINT_CO2);

    day_auto_setpoint_humidity_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(day_auto_setpoint_humidity_queue, TOPIC_DAY_AUTO_SETPOINT_HUMIDITY);

    day_auto_setpoint_temperature_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(day_auto_setpoint_temperature_queue, TOPIC_DAY_AUTO_SETPOINT_TEMPERATURE);

    manual_setpoint_exhaust_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(manual_setpoint_exhaust_queue, TOPIC_MANUAL_SETPOINT_EXHAUST);

    manual_setpoint_heater_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(manual_setpoint_heater_queue, TOPIC_MANUAL_SETPOINT_HEATER);

    manual_setpoint_lamp_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(manual_setpoint_lamp_queue, TOPIC_MANUAL_SETPOINT_LAMP);

    manual_setpoint_recirc_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(manual_setpoint_recirc_queue, TOPIC_MANUAL_SETPOINT_RECIRC);

    night_auto_setpoint_co2_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(night_auto_setpoint_co2_queue, TOPIC_NIGHT_AUTO_SETPOINT_CO2);

    night_auto_setpoint_humidity_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(night_auto_setpoint_humidity_queue, TOPIC_NIGHT_AUTO_SETPOINT_HUMIDITY);

    night_auto_setpoint_temperature_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(night_auto_setpoint_temperature_queue, TOPIC_NIGHT_AUTO_SETPOINT_TEMPERATURE);

    measured_co2_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(measured_temperature_queue, TOPIC_MEASURED_CO2);

    measured_humidity_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(measured_humidity_queue, TOPIC_MEASURED_HUMIDITY);

    measured_temperature_queue = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(measured_temperature_queue, TOPIC_MEASURED_TEMPERATURE);


    BaseType_t ret = xTaskCreate(&task, "controller", 2048, NULL,
            (tskIDLE_PRIORITY + 1), NULL);
    if (ret != pdPASS) {
        ESP_LOGE(tag, "controller_initialize, failed to create task (FATAL)");
    }
}

static void task(void *pvParameter)
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
        if (xQueueReceive(day_auto_setpoint_co2_queue, &message, 0)) {

        }
        if (xQueueReceive(day_auto_setpoint_humidity_queue, &message, 0)) {

        }
        if (xQueueReceive(day_auto_setpoint_temperature_queue, &message, 0)) {

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
            hmi_control_update_pv(&hmi_control_co2, message.double_val);
        }
        if (xQueueReceive(measured_humidity_queue, &message, 0)) {
            hmi_control_update_pv(&hmi_control_humidity, message.double_val);
        }
        if (xQueueReceive(measured_temperature_queue, &message, 0)) {
            hmi_control_update_pv(&hmi_control_temperature, message.double_val - 273.15);
        }
        if (xQueueReceive(night_auto_setpoint_co2_queue, &message, 0)) {

        }
        if (xQueueReceive(night_auto_setpoint_humidity_queue, &message, 0)) {

        }
        if (xQueueReceive(night_auto_setpoint_temperature_queue, &message, 0)) {

        }
        vTaskDelay(1);
    };
}
