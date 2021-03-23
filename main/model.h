// The author disclaims copyright to this source code.
#ifndef _MODEL_H_
#define _MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pubsub.h"

/** Activity indicator (integer) */
extern const char *MODEL_ACTIVITY;

/** Exhaust fan actuator (boolean) */
extern const char *MODEL_EXHAUST;
/** Heater actuator (boolean) */
extern const char *MODEL_HEATER;
/** Lamp actuator (boolean) */
extern const char *MODEL_LAMP;
/** Recirculation fan actuator (boolean) */
extern const char *MODEL_RECIRC;

/** AM2301 status (integer) */
extern const char *MODEL_AM2301_STATUS;
/** AM2301 measurement timestamp (integer) */
extern const char *MODEL_AM2301_TIMESTAMP;

/** Circadian (integer) */
extern const char *MODEL_CIRCADIAN;

/** Automatic control CO2 concentration high (boolean) */
extern const char *MODEL_CO2_HI;
/** Automatic control CO2 concentration low (boolean) */
extern const char *MODEL_CO2_LO;
/** Automatic control humidity high (boolean) */
extern const char *MODEL_HUM_HI;
/** Automatic control humidity low (boolean) */
extern const char *MODEL_HUM_LO;
/** Automatic control temperature high (boolean) */
extern const char *MODEL_TEMP_HI;
/** Automatic control temperature low (boolean) */
extern const char *MODEL_TEMP_LO;

/** Control mode (integer) */
extern const char *MODEL_CONTROL_MODE;

/** Day time CO2 concentration setpoint [ppm] (double) */
extern const char *MODEL_CO2_SV_DAY;
/** Day time humidity setpoint [%] (double) */
extern const char *MODEL_HUM_SV_DAY;
/** Day time temperature setpoint [K] (double) */
extern const char *MODEL_TEMP_SV_DAY;

/* Manual control exhaust fan setpoint (boolean) */
extern const char *MODEL_EXHAUST_SV;
/* Manual control heater setpoint (boolean) */
extern const char *MODEL_HEATER_SV;
/* Manual control lamp setpoint (boolean) */
extern const char *MODEL_LAMP_SV;
/* Manual control recirculation fan setpoint (boolean) */
extern const char *MODEL_RECIRC_SV;

/** Measured CO2 concentration [ppm] (double) */
extern const char *MODEL_CO2_PV;
/** Measured humidity [%] (double) */
extern const char *MODEL_HUM_PV;
/** Measured temperature [K] (double) */
extern const char *MODEL_TEMP_PV;

/** Night time CO2 concentration setpoint [ppm] (double) */
extern const char *MODEL_CO2_SV_NIGHT;
/** Night time humidity setpoint [%] (double) */
extern const char *MODEL_HUM_SV_NIGHT;
/** Night time temperature setpoint [K] (double) */
extern const char *MODEL_TEMP_SV_NIGHT;

/** Time (integer) */
extern const char *MODEL_TIME;

extern pubsub_topic_t model_activity;

extern pubsub_topic_t model_exhaust;
extern pubsub_topic_t model_heater;
extern pubsub_topic_t model_lamp;
extern pubsub_topic_t model_recirc;

extern pubsub_topic_t model_am2301_status;
extern pubsub_topic_t model_am2301_timestamp;

extern pubsub_topic_t model_circadian;

/** Automatic control CO2 concentration high (boolean) */
extern pubsub_topic_t model_co2_hi;
/** Automatic control CO2 concentration low (boolean) */
extern pubsub_topic_t model_co2_lo;
/** Automatic control humidity high (boolean) */
extern pubsub_topic_t model_hum_hi;
/** Automatic control humidity low (boolean) */
extern pubsub_topic_t model_hum_lo;
/** Automatic control temperature high (boolean) */
extern pubsub_topic_t model_temp_hi;
/** Automatic control temperature low (boolean) */
extern pubsub_topic_t model_temp_lo;

extern pubsub_topic_t model_control_mode;

extern pubsub_topic_t model_co2_sv_day;
extern pubsub_topic_t model_hum_sv_day;
extern pubsub_topic_t model_temp_sv_day;

extern pubsub_topic_t model_exhaust_sv;
extern pubsub_topic_t model_heater_sv;
extern pubsub_topic_t model_lamp_sv;
extern pubsub_topic_t model_recirc_sv;

extern pubsub_topic_t model_co2_pv;
extern pubsub_topic_t model_hum_pv;
extern pubsub_topic_t model_temp_pv;

extern pubsub_topic_t model_co2_sv_night;
extern pubsub_topic_t model_hum_sv_night;
extern pubsub_topic_t model_temp_sv_night;

extern pubsub_topic_t model_time;

typedef enum
{
    MODEL_CONTROL_MODE_OFF = 0,
    MODEL_CONTROL_MODE_MANUAL = 1,
    MODEL_CONTROL_MODE_AUTO = 2
} model_control_mode_t;

typedef enum
{
    MODEL_ACTIVE_NO = 0, MODEL_ACTIVE_YES = 1
} model_active_t;

typedef enum
{
    MODEL_COMPONENT_STATUS_OK = 0,
    MODEL_COMPONENT_STATUS_RECOVERABLE = 1,
    MODEL_COMPONENT_STATUS_FATAL = 2
} model_component_status_t;

typedef enum
{
    MODEL_CIRCADIAN_NIGHT = 0, MODEL_CIRCADIAN_DAY = 1
} model_circadian_t;

void model_initialize();

#ifdef __cplusplus
}
#endif

#endif /* _MODEL_H_ */
