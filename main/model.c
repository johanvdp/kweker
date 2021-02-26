// The author disclaims copyright to this source code.
#include "model.h"

const char *TOPIC_ACTIVITY = "led.activity";
const char *TOPIC_ACTUATOR_EXHAUST = "actuator.exhaust";
const char *TOPIC_ACTUATOR_HEATER = "actuator.heater";
const char *TOPIC_ACTUATOR_LAMP = "actuator.lamp";
const char *TOPIC_ACTUATOR_RECIRC = "actuator.recirc";
const char *TOPIC_AM2301_STATUS = "am2301.status";
const char *TOPIC_AM2301_TIMESTAMP = "am2301.timestamp";
const char *TOPIC_CIRCADIAN = "circadian";
const char *TOPIC_CONTROL_MODE = "controlmode";
const char *TOPIC_DAY_AUTO_SETPOINT_CO2 = "day.auto.setpoint.co2";
const char *TOPIC_DAY_AUTO_SETPOINT_HUMIDITY = "day.auto.setpoint.humidity";
const char *TOPIC_DAY_AUTO_SETPOINT_TEMPERATURE =
        "day.auto.setpoint.temperature";
const char *TOPIC_MEASURED_CO2 = "measured.co2";
const char *TOPIC_MEASURED_HUMIDITY = "measured.humidity";
const char *TOPIC_MEASURED_TEMPERATURE = "measured.temperature";
const char *TOPIC_MANUAL_SETPOINT_EXHAUST = "manual.setpoint.exhaust";
const char *TOPIC_MANUAL_SETPOINT_HEATER = "manual.setpoint.heater";
const char *TOPIC_MANUAL_SETPOINT_LAMP = "manual.setpoint.light";
const char *TOPIC_MANUAL_SETPOINT_RECIRC = "manual.setpoint.recirc";
const char *TOPIC_NIGHT_AUTO_SETPOINT_CO2 = "night.auto.setpoint.co2";
const char *TOPIC_NIGHT_AUTO_SETPOINT_HUMIDITY = "night.auto.setpoint.humidity";
const char *TOPIC_NIGHT_AUTO_SETPOINT_TEMPERATURE =
        "night.auto.setpoint.temperature";

/** pulse count (integer) */
pubsub_topic_t activity_topic;
/** ACTUATOR (boolean) */
pubsub_topic_t actuator_exhaust_topic;
/** ACTUATOR (boolean) */
pubsub_topic_t actuator_heater_topic;
/** ACTUATOR (boolean) */
pubsub_topic_t actuator_lamp_topic;
/** ACTUATOR (boolean) */
pubsub_topic_t actuator_recirc_topic;
/** AM2301_STATUS (integer) */
pubsub_topic_t am2301_status_topic;
/** timestamp (integer) */
pubsub_topic_t am2301_timestamp_topic;
/** CIRCADIAN value (integer) */
pubsub_topic_t circadian_topic;
/** CONTROL_MODE value (integer) */
pubsub_topic_t control_mode_topic;
/** CO2 concentration [ppm] (double) */
pubsub_topic_t day_auto_setpoint_co2_topic;
/** relative humidity [%] (double) */
pubsub_topic_t day_auto_setpoint_humidity_topic;
/** temperature [K] (double) */
pubsub_topic_t day_auto_setpoint_temperature_topic;
/** ACTUATOR (boolean) */
pubsub_topic_t manual_setpoint_exhaust_topic;
/** ACTUATOR (boolean) */
pubsub_topic_t manual_setpoint_heater_topic;
/** ACTUATOR (boolean) */
pubsub_topic_t manual_setpoint_light_topic;
/** ACTUATOR (boolean) */
pubsub_topic_t manual_setpoint_recirc_topic;
/** CO2 concentration [ppm] (double) */
pubsub_topic_t measured_co2_topic;
/** relative humidity [%] (double) */
pubsub_topic_t measured_humidity_topic;
/** temperature [K] (double) */
pubsub_topic_t measured_temperature_topic;
/** CO2 concentration [ppm] (double) */
pubsub_topic_t night_auto_setpoint_co2_topic;
/** relative humidity [%] (double) */
pubsub_topic_t night_auto_setpoint_humidity_topic;
/** temperature [K] (double) */
pubsub_topic_t night_auto_setpoint_temperature_topic;

#define ACTUATOR_ON (true)
#define ACTUATOR_OFF (false)

#define AM2301_STATUS_OK (0)
#define AM2301_STATUS_RECOVERABLE (1)
#define AM2301_STATUS_FATAL (2)

#define CIRCADIAN_NIGHT (0)
#define CIRCADIAN_DAY (1)

#define CONTROL_MODE_OFF (0)
#define CONTROL_MODE_MANUAL (1)
#define CONTROL_MODE_AUTO (2)

void model_initialize()
{
    activity_topic = pubsub_register_topic(TOPIC_ACTIVITY);
    actuator_lamp_topic = pubsub_register_topic(TOPIC_ACTUATOR_LAMP);
    actuator_exhaust_topic = pubsub_register_topic(TOPIC_ACTUATOR_EXHAUST);
    actuator_recirc_topic = pubsub_register_topic(TOPIC_ACTUATOR_RECIRC);
    actuator_heater_topic = pubsub_register_topic(TOPIC_ACTUATOR_HEATER);
    am2301_status_topic = pubsub_register_topic(TOPIC_AM2301_STATUS);
    am2301_timestamp_topic = pubsub_register_topic(TOPIC_AM2301_TIMESTAMP);
    circadian_topic = pubsub_register_topic(TOPIC_CIRCADIAN);
    control_mode_topic = pubsub_register_topic(TOPIC_CONTROL_MODE);
    day_auto_setpoint_temperature_topic = pubsub_register_topic(
            TOPIC_DAY_AUTO_SETPOINT_TEMPERATURE);
    day_auto_setpoint_humidity_topic = pubsub_register_topic(
            TOPIC_DAY_AUTO_SETPOINT_HUMIDITY);
    day_auto_setpoint_co2_topic = pubsub_register_topic(
            TOPIC_DAY_AUTO_SETPOINT_CO2);
    manual_setpoint_exhaust_topic = pubsub_register_topic(
            TOPIC_MANUAL_SETPOINT_EXHAUST);
    manual_setpoint_heater_topic = pubsub_register_topic(
            TOPIC_MANUAL_SETPOINT_HEATER);
    manual_setpoint_light_topic = pubsub_register_topic(
            TOPIC_MANUAL_SETPOINT_LAMP);
    manual_setpoint_recirc_topic = pubsub_register_topic(
            TOPIC_MANUAL_SETPOINT_RECIRC);
    measured_temperature_topic = pubsub_register_topic(
            TOPIC_MEASURED_TEMPERATURE);
    measured_humidity_topic = pubsub_register_topic(TOPIC_MEASURED_HUMIDITY);
    measured_co2_topic = pubsub_register_topic(TOPIC_MEASURED_CO2);
    night_auto_setpoint_temperature_topic = pubsub_register_topic(
            TOPIC_NIGHT_AUTO_SETPOINT_TEMPERATURE);
    night_auto_setpoint_humidity_topic = pubsub_register_topic(
            TOPIC_NIGHT_AUTO_SETPOINT_HUMIDITY);
    night_auto_setpoint_co2_topic = pubsub_register_topic(
            TOPIC_NIGHT_AUTO_SETPOINT_CO2);
}
