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

const char *TOPIC_CONTROL_AUTO_CO2_HI = "control.auto.co2.hi";
const char *TOPIC_CONTROL_AUTO_CO2_LO = "control.auto.co2.lo";
const char *TOPIC_CONTROL_AUTO_HUMIDITY_HI = "control.auto.humidity.hi";
const char *TOPIC_CONTROL_AUTO_HUMIDITY_LO = "control.auto.humidity.lo";
const char *TOPIC_CONTROL_AUTO_TEMPERATURE_HI = "control.auto.temperature.hi";
const char *TOPIC_CONTROL_AUTO_TEMPERATURE_LO = "control.auto.temperature.lo";

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

const char *TOPIC_TIME = "time";

pubsub_topic_t activity_topic;

pubsub_topic_t actuator_exhaust_topic;
pubsub_topic_t actuator_heater_topic;
pubsub_topic_t actuator_lamp_topic;
pubsub_topic_t actuator_recirc_topic;

pubsub_topic_t am2301_status_topic;
pubsub_topic_t am2301_timestamp_topic;

pubsub_topic_t circadian_topic;

pubsub_topic_t control_auto_co2_hi_topic;
pubsub_topic_t control_auto_co2_lo_topic;
pubsub_topic_t control_auto_humidity_hi_topic;
pubsub_topic_t control_auto_humidity_lo_topic;
pubsub_topic_t control_auto_temperature_hi_topic;
pubsub_topic_t control_auto_temperature_lo_topic;

pubsub_topic_t control_mode_topic;

pubsub_topic_t day_auto_setpoint_co2_topic;
pubsub_topic_t day_auto_setpoint_humidity_topic;
pubsub_topic_t day_auto_setpoint_temperature_topic;

pubsub_topic_t manual_setpoint_exhaust_topic;
pubsub_topic_t manual_setpoint_heater_topic;
pubsub_topic_t manual_setpoint_light_topic;
pubsub_topic_t manual_setpoint_recirc_topic;

pubsub_topic_t measured_co2_topic;
pubsub_topic_t measured_humidity_topic;
pubsub_topic_t measured_temperature_topic;

pubsub_topic_t night_auto_setpoint_co2_topic;
pubsub_topic_t night_auto_setpoint_humidity_topic;
pubsub_topic_t night_auto_setpoint_temperature_topic;

pubsub_topic_t time_topic;

void model_initialize()
{
    activity_topic = pubsub_register_topic(TOPIC_ACTIVITY, PUBSUB_TYPE_INT);

    actuator_lamp_topic = pubsub_register_topic(TOPIC_ACTUATOR_LAMP,
            PUBSUB_TYPE_INT);
    actuator_exhaust_topic = pubsub_register_topic(TOPIC_ACTUATOR_EXHAUST,
            PUBSUB_TYPE_BOOLEAN);
    actuator_recirc_topic = pubsub_register_topic(TOPIC_ACTUATOR_RECIRC,
            PUBSUB_TYPE_BOOLEAN);
    actuator_heater_topic = pubsub_register_topic(TOPIC_ACTUATOR_HEATER,
            PUBSUB_TYPE_BOOLEAN);

    am2301_status_topic = pubsub_register_topic(TOPIC_AM2301_STATUS,
            PUBSUB_TYPE_INT);
    am2301_timestamp_topic = pubsub_register_topic(TOPIC_AM2301_TIMESTAMP,
            PUBSUB_TYPE_INT);

    circadian_topic = pubsub_register_topic(TOPIC_CIRCADIAN, PUBSUB_TYPE_INT);

    control_mode_topic = pubsub_register_topic(TOPIC_CONTROL_MODE,
            PUBSUB_TYPE_INT);

    control_auto_co2_hi_topic = pubsub_register_topic(TOPIC_CONTROL_AUTO_CO2_HI,
            PUBSUB_TYPE_BOOLEAN);
    control_auto_co2_lo_topic = pubsub_register_topic(TOPIC_CONTROL_AUTO_CO2_LO,
            PUBSUB_TYPE_BOOLEAN);
    control_auto_humidity_hi_topic = pubsub_register_topic(
            TOPIC_CONTROL_AUTO_HUMIDITY_HI, PUBSUB_TYPE_BOOLEAN);
    control_auto_humidity_lo_topic = pubsub_register_topic(
            TOPIC_CONTROL_AUTO_HUMIDITY_LO, PUBSUB_TYPE_BOOLEAN);
    control_auto_temperature_hi_topic = pubsub_register_topic(
            TOPIC_CONTROL_AUTO_TEMPERATURE_HI, PUBSUB_TYPE_BOOLEAN);
    control_auto_temperature_lo_topic = pubsub_register_topic(
            TOPIC_CONTROL_AUTO_TEMPERATURE_LO, PUBSUB_TYPE_BOOLEAN);

    day_auto_setpoint_temperature_topic = pubsub_register_topic(
            TOPIC_DAY_AUTO_SETPOINT_TEMPERATURE, PUBSUB_TYPE_DOUBLE);
    day_auto_setpoint_humidity_topic = pubsub_register_topic(
            TOPIC_DAY_AUTO_SETPOINT_HUMIDITY, PUBSUB_TYPE_DOUBLE);
    day_auto_setpoint_co2_topic = pubsub_register_topic(
            TOPIC_DAY_AUTO_SETPOINT_CO2, PUBSUB_TYPE_DOUBLE);

    manual_setpoint_exhaust_topic = pubsub_register_topic(
            TOPIC_MANUAL_SETPOINT_EXHAUST, PUBSUB_TYPE_BOOLEAN);
    manual_setpoint_heater_topic = pubsub_register_topic(
            TOPIC_MANUAL_SETPOINT_HEATER, PUBSUB_TYPE_BOOLEAN);
    manual_setpoint_light_topic = pubsub_register_topic(
            TOPIC_MANUAL_SETPOINT_LAMP, PUBSUB_TYPE_BOOLEAN);
    manual_setpoint_recirc_topic = pubsub_register_topic(
            TOPIC_MANUAL_SETPOINT_RECIRC, PUBSUB_TYPE_BOOLEAN);

    measured_temperature_topic = pubsub_register_topic(
            TOPIC_MEASURED_TEMPERATURE, PUBSUB_TYPE_DOUBLE);
    measured_humidity_topic = pubsub_register_topic(TOPIC_MEASURED_HUMIDITY,
            PUBSUB_TYPE_DOUBLE);
    measured_co2_topic = pubsub_register_topic(TOPIC_MEASURED_CO2,
            PUBSUB_TYPE_DOUBLE);

    night_auto_setpoint_temperature_topic = pubsub_register_topic(
            TOPIC_NIGHT_AUTO_SETPOINT_TEMPERATURE, PUBSUB_TYPE_DOUBLE);
    night_auto_setpoint_humidity_topic = pubsub_register_topic(
            TOPIC_NIGHT_AUTO_SETPOINT_HUMIDITY, PUBSUB_TYPE_DOUBLE);
    night_auto_setpoint_co2_topic = pubsub_register_topic(
            TOPIC_NIGHT_AUTO_SETPOINT_CO2, PUBSUB_TYPE_DOUBLE);

    time_topic = pubsub_register_topic(TOPIC_TIME, PUBSUB_TYPE_INT);
}
