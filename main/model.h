// The author disclaims copyright to this source code.
#ifndef _MODEL_H_
#define _MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pubsub.h"

extern const char *TOPIC_ACTIVITY;
extern const char *TOPIC_ACTUATOR_EXHAUST;
extern const char *TOPIC_ACTUATOR_HEATER;
extern const char *TOPIC_ACTUATOR_LAMP;
extern const char *TOPIC_ACTUATOR_RECIRC;
extern const char *TOPIC_AM2301_STATUS;
extern const char *TOPIC_AM2301_TIMESTAMP;
extern const char *TOPIC_CIRCADIAN;
extern const char *TOPIC_CONTROL_MODE;
extern const char *TOPIC_DAY_AUTO_SETPOINT_CO2;
extern const char *TOPIC_DAY_AUTO_SETPOINT_HUMIDITY;
extern const char *TOPIC_DAY_AUTO_SETPOINT_TEMPERATURE;
extern const char *TOPIC_MANUAL_SETPOINT_EXHAUST;
extern const char *TOPIC_MANUAL_SETPOINT_HEATER;
extern const char *TOPIC_MANUAL_SETPOINT_LAMP;
extern const char *TOPIC_MANUAL_SETPOINT_RECIRC;
extern const char *TOPIC_MEASURED_CO2;
extern const char *TOPIC_MEASURED_HUMIDITY;
extern const char *TOPIC_MEASURED_TEMPERATURE;
extern const char *TOPIC_NIGHT_AUTO_SETPOINT_CO2;
extern const char *TOPIC_NIGHT_AUTO_SETPOINT_HUMIDITY;
extern const char *TOPIC_NIGHT_AUTO_SETPOINT_TEMPERATURE;


extern pubsub_topic_t activity_topic;
extern pubsub_topic_t actuator_exhaust_topic;
extern pubsub_topic_t actuator_heater_topic;
extern pubsub_topic_t actuator_lamp_topic;
extern pubsub_topic_t actuator_recirc_topic;
extern pubsub_topic_t am2301_status_topic;
extern pubsub_topic_t am2301_timestamp_topic;
extern pubsub_topic_t circadian_topic;
extern pubsub_topic_t control_mode_topic;
extern pubsub_topic_t day_auto_setpoint_co2_topic;
extern pubsub_topic_t day_auto_setpoint_humidity_topic;
extern pubsub_topic_t day_auto_setpoint_temperature_topic;
extern pubsub_topic_t manual_setpoint_exhaust_topic;
extern pubsub_topic_t manual_setpoint_heater_topic;
extern pubsub_topic_t manual_setpoint_lamp_topic;
extern pubsub_topic_t manual_setpoint_recirc_topic;
extern pubsub_topic_t measured_co2_topic;
extern pubsub_topic_t measured_humidity_topic;
extern pubsub_topic_t measured_temperature_topic;
extern pubsub_topic_t night_auto_setpoint_co2_topic;
extern pubsub_topic_t night_auto_setpoint_humidity_topic;
extern pubsub_topic_t night_auto_setpoint_temperature_topic;

void model_initialize();

#ifdef __cplusplus
}
#endif

#endif /* _MODEL_H_ */
