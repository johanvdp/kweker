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

/** Exhaust fan actuator (boolean) */
extern const char *MODEL_EXHAUST;

/** Heater actuator (boolean) */
extern const char *MODEL_HEATER;

/** Light actuator (boolean) */
extern const char *MODEL_LIGHT;

/** Recirculation fan actuator (boolean) */
extern const char *MODEL_RECIRC;

/******************
 * sensor state
 */

/** Current time in seconds after epoch (integer, time_t) */
extern const char *MODEL_CURRENT_TIME;

/** AM2301 status (integer, model_component_status_t) */
extern const char *MODEL_AM2301_STATUS;

/** AM2301 measurement timestamp (integer) */
extern const char *MODEL_AM2301_TIMESTAMP;

/** Measured CO2 concentration [ppm] (double) */
extern const char *MODEL_CO2_PV;

/** Measured humidity [%] (double) */
extern const char *MODEL_HUM_PV;

/** Measured temperature [K] (double) */
extern const char *MODEL_TEMP_PV;

/******************
 * controller state
 */

/** Control mode (integer, model_control_mode_t) */
extern const char *MODEL_CONTROL_MODE;

/** Circadian (integer, model_circadian_t) */
extern const char *MODEL_CIRCADIAN;

/** Current CO2 concentration setpoint [ppm] (double) */
extern const char *MODEL_CO2_SV;

/** Current humidity setpoint [%] (double) */
extern const char *MODEL_HUM_SV;

/** Current temperature setpoint [K] (double) */
extern const char *MODEL_TEMP_SV;

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
extern const char *MODEL_BEGIN_OF_NIGHT;

/** Day time CO2 concentration setpoint [ppm] (double) */
extern const char *MODEL_CO2_SV_DAY;

/** Night time CO2 concentration setpoint [ppm] (double) */
extern const char *MODEL_CO2_SV_NIGHT;

/** Day time humidity setpoint [%] (double) */
extern const char *MODEL_HUM_SV_DAY;

/** Night time humidity setpoint [%] (double) */
extern const char *MODEL_HUM_SV_NIGHT;

/** Day time temperature setpoint [K] (double) */
extern const char *MODEL_TEMP_SV_DAY;

/** Night time temperature setpoint [K] (double) */
extern const char *MODEL_TEMP_SV_NIGHT;

/** Manual control exhaust fan setpoint (boolean) */
extern const char *MODEL_EXHAUST_SV;

/** Manual control heater setpoint (boolean) */
extern const char *MODEL_HEATER_SV;

/** Manual control light setpoint (boolean) */
extern const char *MODEL_LIGHT_SV;

/** Manual control recirculation fan setpoint (boolean) */
extern const char *MODEL_RECIRC_SV;

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
