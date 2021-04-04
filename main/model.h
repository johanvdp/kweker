// The author disclaims copyright to this source code.

#ifndef _MODEL_H_
#define _MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pubsub.h"


/******************
 * actuator state
 */

/** Activity indicator (integer) */
extern const char *MODEL_ACTIVITY;
extern pubsub_topic_t model_activity;

/** Exhaust fan actuator (boolean) */
extern const char *MODEL_EXHAUST;
extern pubsub_topic_t model_exhaust;

/** Heater actuator (boolean) */
extern const char *MODEL_HEATER;
extern pubsub_topic_t model_heater;

/** Light actuator (boolean) */
extern const char *MODEL_LIGHT;
extern pubsub_topic_t model_light;

/** Recirculation fan actuator (boolean) */
extern const char *MODEL_RECIRC;
extern pubsub_topic_t model_recirc;

/******************
 * sensor state
 */

/** Current time in seconds after epoch (integer, time_t) */
extern const char *MODEL_CURRENT_TIME;
extern pubsub_topic_t model_current_time;

/** AM2301 status (integer, model_component_status_t) */
extern const char *MODEL_AM2301_STATUS;
extern pubsub_topic_t model_am2301_status;

/** AM2301 measurement timestamp (integer) */
extern const char *MODEL_AM2301_TIMESTAMP;
extern pubsub_topic_t model_am2301_timestamp;

/** Measured CO2 concentration [ppm] (double) */
extern const char *MODEL_CO2_PV;
extern pubsub_topic_t model_co2_pv;

/** Measured humidity [%] (double) */
extern const char *MODEL_HUM_PV;
extern pubsub_topic_t model_hum_pv;

/** Measured temperature [K] (double) */
extern const char *MODEL_TEMP_PV;
extern pubsub_topic_t model_temp_pv;


/******************
 * controller state
 */

/** Control mode (integer, model_control_mode_t) */
extern const char *MODEL_CONTROL_MODE;
/** Control mode (integer, model_control_mode_t) */
extern pubsub_topic_t model_control_mode;

/** Circadian (integer, model_circadian_t) */
extern const char *MODEL_CIRCADIAN;
/** Circadian (integer, model_circadian_t) */
extern pubsub_topic_t model_circadian;

/** Current CO2 concentration setpoint [ppm] (double) */
extern const char *MODEL_CO2_SV;
/** Current CO2 concentration setpoint [ppm] (double) */
extern pubsub_topic_t model_co2_sv;

/** Current humidity setpoint [%] (double) */
extern const char *MODEL_HUM_SV;
/** Current humidity setpoint [%] (double) */
extern pubsub_topic_t model_hum_sv;

/** Current temperature setpoint [K] (double) */
extern const char *MODEL_TEMP_SV;
/** Current temperature setpoint [K] (double) */
extern pubsub_topic_t model_temp_sv;

/** Automatic control CO2 concentration high (boolean) */
extern const char *MODEL_CO2_HI;
/** Automatic control CO2 concentration high (boolean) */
extern pubsub_topic_t model_co2_hi;

/** Automatic control CO2 concentration low (boolean) */
extern const char *MODEL_CO2_LO;
/** Automatic control CO2 concentration low (boolean) */
extern pubsub_topic_t model_co2_lo;

/** Automatic control humidity high (boolean) */
extern const char *MODEL_HUM_HI;
/** Automatic control humidity high (boolean) */
extern pubsub_topic_t model_hum_hi;

/** Automatic control humidity low (boolean) */
extern const char *MODEL_HUM_LO;
/** Automatic control humidity low (boolean) */
extern pubsub_topic_t model_hum_lo;

/** Automatic control temperature high (boolean) */
extern const char *MODEL_TEMP_HI;
/** Automatic control temperature high (boolean) */
extern pubsub_topic_t model_temp_hi;

/** Automatic control temperature low (boolean) */
extern const char *MODEL_TEMP_LO;
/** Automatic control temperature low (boolean) */
extern pubsub_topic_t model_temp_lo;


/******************
 * setpoints
 * (user settings)
 */

/**
 * Begin of day in seconds after epoch (time_t)
 * always in the same day (2000-01-01)
 */
extern const char *MODEL_BEGIN_OF_DAY;
/**
 * Begin of day in seconds after epoch (time_t)
 * always in the same day (2000-01-01)
 */
extern pubsub_topic_t model_begin_of_day;

/**
 * Begin of day in seconds after epoch (time_t)
 * always in the same day (2000-01-01)
 */
extern const char *MODEL_BEGIN_OF_NIGHT;
/**
 * Begin of day in seconds after epoch (time_t)
 * always in the same day (2000-01-01)
 */
extern pubsub_topic_t model_begin_of_night;

/** Day time CO2 concentration setpoint [ppm] (double) */
extern const char *MODEL_CO2_SV_DAY;
/** Day time CO2 concentration setpoint [ppm] (double) */
extern pubsub_topic_t model_co2_sv_day;

/** Night time CO2 concentration setpoint [ppm] (double) */
extern const char *MODEL_CO2_SV_NIGHT;
/** Night time CO2 concentration setpoint [ppm] (double) */
extern pubsub_topic_t model_co2_sv_night;

/** Day time humidity setpoint [%] (double) */
extern const char *MODEL_HUM_SV_DAY;
/** Day time humidity setpoint [%] (double) */
extern pubsub_topic_t model_hum_sv_day;

/** Night time humidity setpoint [%] (double) */
extern const char *MODEL_HUM_SV_NIGHT;
/** Night time humidity setpoint [%] (double) */
extern pubsub_topic_t model_hum_sv_night;

/** Day time temperature setpoint [K] (double) */
extern const char *MODEL_TEMP_SV_DAY;
/** Day time temperature setpoint [K] (double) */
extern pubsub_topic_t model_temp_sv_day;

/** Night time temperature setpoint [K] (double) */
extern const char *MODEL_TEMP_SV_NIGHT;
/** Night time temperature setpoint [K] (double) */
extern pubsub_topic_t model_temp_sv_night;

/** Manual control exhaust fan setpoint (boolean) */
extern const char *MODEL_EXHAUST_SV;
/** Manual control exhaust fan setpoint (boolean) */
extern pubsub_topic_t model_exhaust_sv;

/** Manual control heater setpoint (boolean) */
extern const char *MODEL_HEATER_SV;
/** Manual control heater setpoint (boolean) */
extern pubsub_topic_t model_heater_sv;

/** Manual control light setpoint (boolean) */
extern const char *MODEL_LIGHT_SV;
/** Manual control light setpoint (boolean) */
extern pubsub_topic_t model_light_sv;

/** Manual control recirculation fan setpoint (boolean) */
extern const char *MODEL_RECIRC_SV;
/** Manual control recirculation fan setpoint (boolean) */
extern pubsub_topic_t model_recirc_sv;

/** Control mode */
typedef enum
{
    MODEL_CONTROL_MODE_OFF = 0, MODEL_CONTROL_MODE_MANUAL = 1, MODEL_CONTROL_MODE_AUTO = 2
} model_control_mode_t;

/** Component status */
typedef enum
{
    MODEL_COMPONENT_STATUS_OK = 0, MODEL_COMPONENT_STATUS_RECOVERABLE = 1, MODEL_COMPONENT_STATUS_FATAL = 2
} model_component_status_t;

/** Circadian phase */
typedef enum
{
    MODEL_CIRCADIAN_NIGHT = 0, MODEL_CIRCADIAN_DAY = 1
} model_circadian_t;

void model_initialize();

#ifdef __cplusplus
}
#endif

#endif /* _MODEL_H_ */
