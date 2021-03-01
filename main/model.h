// The author disclaims copyright to this source code.
#ifndef _MODEL_H_
#define _MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pubsub.h"

/** pulse count (integer) */
extern const char *TOPIC_ACTIVITY;

/** exhaust fan actuator (boolean) */
extern const char *TOPIC_ACTUATOR_EXHAUST;
/** heater actuator (boolean) */
extern const char *TOPIC_ACTUATOR_HEATER;
/** lamp actuator (boolean) */
extern const char *TOPIC_ACTUATOR_LAMP;
/** recirculation fan actuator (boolean) */
extern const char *TOPIC_ACTUATOR_RECIRC;

/** AM2301 status (integer) */
extern const char *TOPIC_AM2301_STATUS;
/** AM2301 measurement timestamp (integer) */
extern const char *TOPIC_AM2301_TIMESTAMP;

/** Circadian (integer) */
extern const char *TOPIC_CIRCADIAN;

/** Automatic control CO2 concentration high (boolean) */
extern const char *TOPIC_CONTROL_AUTO_CO2_HI;
/** Automatic control CO2 concentration low (boolean) */
extern const char *TOPIC_CONTROL_AUTO_CO2_LO;
/** Automatic control humidity high (boolean) */
extern const char *TOPIC_CONTROL_AUTO_HUMIDITY_HI;
/** Automatic control humidity low (boolean) */
extern const char *TOPIC_CONTROL_AUTO_HUMIDITY_LO;
/** Automatic control temperature high (boolean) */
extern const char *TOPIC_CONTROL_AUTO_TEMPERATURE_HI;
/** Automatic control temperature low (boolean) */
extern const char *TOPIC_CONTROL_AUTO_TEMPERATURE_LO;

/** Control mode (integer) */
extern const char *TOPIC_CONTROL_MODE;

/** Day time CO2 concentration setpoint [ppm] (double) */
extern const char *TOPIC_DAY_AUTO_SETPOINT_CO2;
/** Day time humidity setpoint [%] (double) */
extern const char *TOPIC_DAY_AUTO_SETPOINT_HUMIDITY;
/** Day time temperature setpoint [K] (double) */
extern const char *TOPIC_DAY_AUTO_SETPOINT_TEMPERATURE;

/* Manual control exhaust fan setpoint (boolean) */
extern const char *TOPIC_MANUAL_SETPOINT_EXHAUST;
/* Manual control heater setpoint (boolean) */
extern const char *TOPIC_MANUAL_SETPOINT_HEATER;
/* Manual control lamp setpoint (boolean) */
extern const char *TOPIC_MANUAL_SETPOINT_LAMP;
/* Manual control recirculation fan setpoint (boolean) */
extern const char *TOPIC_MANUAL_SETPOINT_RECIRC;

/** Measured CO2 concentration [ppm] (double) */
extern const char *TOPIC_MEASURED_CO2;
/** Measured humidity [%] (double) */
extern const char *TOPIC_MEASURED_HUMIDITY;
/** Measured temperature [K] (double) */
extern const char *TOPIC_MEASURED_TEMPERATURE;

/** Night time CO2 concentration setpoint [ppm] (double) */
extern const char *TOPIC_NIGHT_AUTO_SETPOINT_CO2;
/** Night time humidity setpoint [%] (double) */
extern const char *TOPIC_NIGHT_AUTO_SETPOINT_HUMIDITY;
/** Night time temperature setpoint [K] (double) */
extern const char *TOPIC_NIGHT_AUTO_SETPOINT_TEMPERATURE;


extern pubsub_topic_t activity_topic;

extern pubsub_topic_t actuator_exhaust_topic;
extern pubsub_topic_t actuator_heater_topic;
extern pubsub_topic_t actuator_lamp_topic;
extern pubsub_topic_t actuator_recirc_topic;

extern pubsub_topic_t am2301_status_topic;
extern pubsub_topic_t am2301_timestamp_topic;

extern pubsub_topic_t circadian_topic;

/** Automatic control CO2 concentration high (boolean) */
extern pubsub_topic_t control_auto_co2_hi_topic;
/** Automatic control CO2 concentration low (boolean) */
extern pubsub_topic_t control_auto_co2_lo_topic;
/** Automatic control humidity high (boolean) */
extern pubsub_topic_t control_auto_humidity_hi_topic;
/** Automatic control humidity low (boolean) */
extern pubsub_topic_t control_auto_humidity_lo_topic;
/** Automatic control temperature high (boolean) */
extern pubsub_topic_t control_auto_temperature_hi_topic;
/** Automatic control temperature low (boolean) */
extern pubsub_topic_t control_auto_temperature_lo_topic;

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


typedef enum
{
    HMI_CONTROL_MODE_OFF = 0, HMI_CONTROL_MODE_MANUAL = 1, HMI_CONTROL_MODE_AUTO = 2
} model_control_mode_t;

typedef enum
{
    HMI_ACTIVE_NO = 0, HMI_ACTIVE_YES = 1
} model_active_t;

typedef enum
{
    HMI_COMPONENT_STATUS_OK = 0, HMI_COMPONENT_STATUS_RECOVERABLE = 1, HMI_COMPONENT_STATUS_FATAL = 2
} model_component_status_t;

typedef enum
{
    HMI_CIRCADIAN_NIGHT = 0, HMI_CIRCADIAN_DAY = 1
} model_circadian_t;


void model_initialize();

#ifdef __cplusplus
}
#endif

#endif /* _MODEL_H_ */
