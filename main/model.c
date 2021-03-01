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

    control_auto_co2_hi_topic = pubsub_register_topic(
            TOPIC_CONTROL_AUTO_CO2_HI);
    control_auto_co2_lo_topic = pubsub_register_topic(
            TOPIC_CONTROL_AUTO_CO2_LO);
    control_auto_humidity_hi_topic = pubsub_register_topic(
            TOPIC_CONTROL_AUTO_HUMIDITY_HI);
    control_auto_humidity_lo_topic = pubsub_register_topic(
            TOPIC_CONTROL_AUTO_HUMIDITY_LO);
    control_auto_temperature_hi_topic = pubsub_register_topic(
            TOPIC_CONTROL_AUTO_TEMPERATURE_HI);
    control_auto_temperature_lo_topic = pubsub_register_topic(
            TOPIC_CONTROL_AUTO_TEMPERATURE_LO);

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
