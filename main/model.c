// The author disclaims copyright to this source code.
#include "model.h"

const char *TOPIC_ACTIVITY = "led.activity";
const char *TOPIC_CURRENT_TEMPERATURE = "am2301.temperature";
const char *TOPIC_CURRENT_HUMIDITY = "am2301.humidity";
const char *TOPIC_AM2301_STATUS = "am2301.status";
const char *TOPIC_AM2301_TIMESTAMP = "am2301.timestamp";
const char *TOPIC_ACTUATOR_LAMP = "do.lamp";
const char *TOPIC_ACTUATOR_EXHAUST = "do.exhaust";
const char *TOPIC_ACTUATOR_RECIRC = "do.recirc";
const char *TOPIC_ACTUATOR_HEATER = "do.heater";

pubsub_topic_t activity_topic;
pubsub_topic_t current_temperature_topic;
pubsub_topic_t current_humidity_topic;
pubsub_topic_t am2301_status_topic;
pubsub_topic_t am2301_timestamp_topic;
pubsub_topic_t actuator_lamp_topic;
pubsub_topic_t actuator_exhaust_topic;
pubsub_topic_t actuator_recirc_topic;
pubsub_topic_t actuator_heater_topic;

void model_initialize()
{
    activity_topic = pubsub_register_topic(TOPIC_ACTIVITY);
    current_temperature_topic = pubsub_register_topic(TOPIC_CURRENT_TEMPERATURE);
    current_humidity_topic = pubsub_register_topic(TOPIC_CURRENT_HUMIDITY);
    am2301_status_topic = pubsub_register_topic(TOPIC_AM2301_STATUS);
    am2301_timestamp_topic = pubsub_register_topic(
            TOPIC_AM2301_TIMESTAMP);
    actuator_lamp_topic = pubsub_register_topic(TOPIC_ACTUATOR_LAMP);
    actuator_exhaust_topic = pubsub_register_topic(TOPIC_ACTUATOR_EXHAUST);
    actuator_recirc_topic = pubsub_register_topic(TOPIC_ACTUATOR_RECIRC);
    actuator_heater_topic = pubsub_register_topic(TOPIC_ACTUATOR_HEATER);
}
