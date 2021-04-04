// The author disclaims copyright to this source code.

#include "model.h"

/******************
 * actuator state
 */

const char *MODEL_ACTIVITY = "activity";
pubsub_topic_t model_activity;

const char *MODEL_EXHAUST = "exhaust.out";
pubsub_topic_t model_exhaust;

const char *MODEL_HEATER = "heater.out";
pubsub_topic_t model_heater;

const char *MODEL_LIGHT = "light.out";
pubsub_topic_t model_light;

const char *MODEL_RECIRC = "recirc.out";
pubsub_topic_t model_recirc;

/******************
 * sensor state
 */

const char *MODEL_CURRENT_TIME = "time";
pubsub_topic_t model_current_time;

const char *MODEL_AM2301_STATUS = "am2301.status";
pubsub_topic_t model_am2301_status;

const char *MODEL_AM2301_TIMESTAMP = "am2301.time";
pubsub_topic_t model_am2301_timestamp;

const char *MODEL_CO2_PV = "co2.pv";
pubsub_topic_t model_co2_pv;

const char *MODEL_HUM_PV = "hum.pv";
pubsub_topic_t model_hum_pv;

const char *MODEL_TEMP_PV = "temp.pv";
pubsub_topic_t model_temp_pv;

/******************
 * controller state
 */

const char *MODEL_CONTROL_MODE = "control.mode";
pubsub_topic_t model_control_mode;

const char *MODEL_CIRCADIAN = "circadian";
pubsub_topic_t model_circadian;

const char *MODEL_CO2_SV = "co2.sv";
pubsub_topic_t model_co2_sv_day;

const char *MODEL_HUM_SV = "hum.sv";
pubsub_topic_t model_hum_sv_day;

const char *MODEL_TEMP_SV = "temp.sv";
pubsub_topic_t model_temp_sv_day;

const char *MODEL_CO2_HI = "co2.hi";
pubsub_topic_t model_co2_hi;

const char *MODEL_CO2_LO = "co2.lo";
pubsub_topic_t model_co2_lo;

const char *MODEL_HUM_HI = "hum.hi";
pubsub_topic_t model_hum_hi;

const char *MODEL_HUM_LO = "hum.lo";
pubsub_topic_t model_hum_lo;

const char *MODEL_TEMP_HI = "temp.hi";
pubsub_topic_t model_temp_hi;

const char *MODEL_TEMP_LO = "temp.lo";
pubsub_topic_t model_temp_lo;

/******************
 * setpoints
 * (user settings)
 */

const char *MODEL_BEGIN_OF_DAY = "day";
pubsub_topic_t model_begin_of_day;

const char *MODEL_BEGIN_OF_NIGHT = "night";
pubsub_topic_t model_begin_of_night;

const char *MODEL_CO2_SV_DAY = "co2.sv.day";
pubsub_topic_t model_co2_sv;

const char *MODEL_CO2_SV_NIGHT = "co2.sv.night";
pubsub_topic_t model_co2_sv_night;

const char *MODEL_HUM_SV_DAY = "hum.sv.day";
pubsub_topic_t model_hum_sv;

const char *MODEL_HUM_SV_NIGHT = "hum.sv.night";
pubsub_topic_t model_hum_sv_night;

const char *MODEL_TEMP_SV_DAY = "temp.sv.day";
pubsub_topic_t model_temp_sv;

const char *MODEL_TEMP_SV_NIGHT = "temp.sv.night";
pubsub_topic_t model_temp_sv_night;

const char *MODEL_EXHAUST_SV = "exhaust.sv";
pubsub_topic_t model_exhaust_sv;

const char *MODEL_HEATER_SV = "heater.sv";
pubsub_topic_t model_heater_sv;

const char *MODEL_LIGHT_SV = "light.sv";
pubsub_topic_t model_light_sv;

const char *MODEL_RECIRC_SV = "recirc.sv";
pubsub_topic_t model_recirc_sv;


void model_initialize()
{
    model_activity = pubsub_register_topic(MODEL_ACTIVITY, PUBSUB_TYPE_INT, true);

    model_light = pubsub_register_topic(MODEL_LIGHT, PUBSUB_TYPE_BOOLEAN, false);
    model_exhaust = pubsub_register_topic(MODEL_EXHAUST, PUBSUB_TYPE_BOOLEAN, false);
    model_recirc = pubsub_register_topic(MODEL_RECIRC, PUBSUB_TYPE_BOOLEAN, false);
    model_heater = pubsub_register_topic(MODEL_HEATER, PUBSUB_TYPE_BOOLEAN, false);

    model_am2301_status = pubsub_register_topic(MODEL_AM2301_STATUS, PUBSUB_TYPE_INT, true);
    model_am2301_timestamp = pubsub_register_topic(MODEL_AM2301_TIMESTAMP, PUBSUB_TYPE_INT, true);

    model_circadian = pubsub_register_topic(MODEL_CIRCADIAN, PUBSUB_TYPE_INT, false);

    model_control_mode = pubsub_register_topic(MODEL_CONTROL_MODE, PUBSUB_TYPE_INT, false);

    model_co2_hi = pubsub_register_topic(MODEL_CO2_HI, PUBSUB_TYPE_BOOLEAN, false);
    model_co2_lo = pubsub_register_topic(MODEL_CO2_LO, PUBSUB_TYPE_BOOLEAN, false);
    model_hum_hi = pubsub_register_topic(MODEL_HUM_HI, PUBSUB_TYPE_BOOLEAN, false);
    model_hum_lo = pubsub_register_topic(MODEL_HUM_LO, PUBSUB_TYPE_BOOLEAN, false);
    model_temp_hi = pubsub_register_topic(MODEL_TEMP_HI, PUBSUB_TYPE_BOOLEAN, false);
    model_temp_lo = pubsub_register_topic(MODEL_TEMP_LO, PUBSUB_TYPE_BOOLEAN, false);

    model_temp_sv_day = pubsub_register_topic(MODEL_TEMP_SV_DAY, PUBSUB_TYPE_DOUBLE, false);
    model_hum_sv_day = pubsub_register_topic(MODEL_HUM_SV_DAY, PUBSUB_TYPE_DOUBLE, false);
    model_co2_sv_day = pubsub_register_topic(MODEL_CO2_SV_DAY, PUBSUB_TYPE_DOUBLE, false);

    model_exhaust_sv = pubsub_register_topic(MODEL_EXHAUST_SV, PUBSUB_TYPE_BOOLEAN, false);
    model_heater_sv = pubsub_register_topic(MODEL_HEATER_SV, PUBSUB_TYPE_BOOLEAN, false);
    model_light_sv = pubsub_register_topic(MODEL_LIGHT_SV, PUBSUB_TYPE_BOOLEAN, false);
    model_recirc_sv = pubsub_register_topic(MODEL_RECIRC_SV, PUBSUB_TYPE_BOOLEAN, false);

    model_temp_pv = pubsub_register_topic(MODEL_TEMP_PV, PUBSUB_TYPE_DOUBLE, true);
    model_hum_pv = pubsub_register_topic(MODEL_HUM_PV, PUBSUB_TYPE_DOUBLE, true);
    model_co2_pv = pubsub_register_topic(MODEL_CO2_PV, PUBSUB_TYPE_DOUBLE, true);

    model_temp_sv_night = pubsub_register_topic(MODEL_TEMP_SV_NIGHT, PUBSUB_TYPE_DOUBLE, false);
    model_hum_sv_night = pubsub_register_topic(MODEL_HUM_SV_NIGHT, PUBSUB_TYPE_DOUBLE, false);
    model_co2_sv_night = pubsub_register_topic(MODEL_CO2_SV_NIGHT, PUBSUB_TYPE_DOUBLE, false);

    model_current_time = pubsub_register_topic(MODEL_CURRENT_TIME, PUBSUB_TYPE_INT, true);
    model_begin_of_day = pubsub_register_topic(MODEL_BEGIN_OF_DAY, PUBSUB_TYPE_INT, false);
    model_begin_of_night = pubsub_register_topic(MODEL_BEGIN_OF_NIGHT, PUBSUB_TYPE_INT, false);

    model_temp_sv = pubsub_register_topic(MODEL_TEMP_SV, PUBSUB_TYPE_DOUBLE, false);
    model_hum_sv = pubsub_register_topic(MODEL_HUM_SV, PUBSUB_TYPE_DOUBLE, false);
    model_co2_sv = pubsub_register_topic(MODEL_CO2_SV, PUBSUB_TYPE_DOUBLE, false);
}
