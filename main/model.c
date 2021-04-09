// The author disclaims copyright to this source code.

#include "model.h"

/******************
 * actuator state
 */

const char *MODEL_ACTIVITY = "activity";

const char *MODEL_EXHAUST = "exhaust.out";
const char *MODEL_HEATER = "heater.out";
const char *MODEL_LIGHT = "light.out";
const char *MODEL_RECIRC = "recirc.out";

/******************
 * sensor state
 */

const char *MODEL_CURRENT_TIME = "time";

const char *MODEL_AM2301_STATUS = "am2301.status";
const char *MODEL_AM2301_TIMESTAMP = "am2301.time";

const char *MODEL_CO2_PV = "co2.pv";
const char *MODEL_HUM_PV = "hum.pv";
const char *MODEL_TEMP_PV = "temp.pv";

/******************
 * controller state
 */

const char *MODEL_CONTROL_MODE = "control.mode";

const char *MODEL_CIRCADIAN = "circadian";

const char *MODEL_CO2_SV = "co2.sv";
const char *MODEL_HUM_SV = "hum.sv";
const char *MODEL_TEMP_SV = "temp.sv";

const char *MODEL_CO2_HI = "co2.hi";
const char *MODEL_CO2_LO = "co2.lo";

const char *MODEL_HUM_HI = "hum.hi";
const char *MODEL_HUM_LO = "hum.lo";

const char *MODEL_TEMP_HI = "temp.hi";
const char *MODEL_TEMP_LO = "temp.lo";

/******************
 * setpoints
 * (user settings)
 */

const char *MODEL_BEGIN_OF_DAY = "day";
const char *MODEL_BEGIN_OF_NIGHT = "night";

const char *MODEL_CO2_SV_DAY = "co2.sv.day";
const char *MODEL_CO2_SV_NIGHT = "co2.sv.night";

const char *MODEL_HUM_SV_DAY = "hum.sv.day";
const char *MODEL_HUM_SV_NIGHT = "hum.sv.night";

const char *MODEL_TEMP_SV_DAY = "temp.sv.day";
const char *MODEL_TEMP_SV_NIGHT = "temp.sv.night";

const char *MODEL_EXHAUST_SV = "exhaust.sv";
const char *MODEL_HEATER_SV = "heater.sv";
const char *MODEL_LIGHT_SV = "light.sv";
const char *MODEL_RECIRC_SV = "recirc.sv";

void model_initialize()
{
    pubsub_register_topic(MODEL_ACTIVITY, PUBSUB_TYPE_INT, true);

    pubsub_register_topic(MODEL_LIGHT, PUBSUB_TYPE_BOOLEAN, false);
    pubsub_register_topic(MODEL_EXHAUST, PUBSUB_TYPE_BOOLEAN, false);
    pubsub_register_topic(MODEL_RECIRC, PUBSUB_TYPE_BOOLEAN, false);
    pubsub_register_topic(MODEL_HEATER, PUBSUB_TYPE_BOOLEAN, false);

    pubsub_register_topic(MODEL_AM2301_STATUS, PUBSUB_TYPE_INT, true);
    pubsub_register_topic(MODEL_AM2301_TIMESTAMP, PUBSUB_TYPE_INT, true);

    pubsub_register_topic(MODEL_CIRCADIAN, PUBSUB_TYPE_INT, false);

    pubsub_register_topic(MODEL_CONTROL_MODE, PUBSUB_TYPE_INT, false);

    pubsub_register_topic(MODEL_CO2_HI, PUBSUB_TYPE_BOOLEAN, false);
    pubsub_register_topic(MODEL_CO2_LO, PUBSUB_TYPE_BOOLEAN, false);
    pubsub_register_topic(MODEL_HUM_HI, PUBSUB_TYPE_BOOLEAN, false);
    pubsub_register_topic(MODEL_HUM_LO, PUBSUB_TYPE_BOOLEAN, false);
    pubsub_register_topic(MODEL_TEMP_HI, PUBSUB_TYPE_BOOLEAN, false);
    pubsub_register_topic(MODEL_TEMP_LO, PUBSUB_TYPE_BOOLEAN, false);

    pubsub_register_topic(MODEL_TEMP_SV_DAY, PUBSUB_TYPE_DOUBLE, false);
    pubsub_register_topic(MODEL_HUM_SV_DAY, PUBSUB_TYPE_DOUBLE, false);
    pubsub_register_topic(MODEL_CO2_SV_DAY, PUBSUB_TYPE_DOUBLE, false);

    pubsub_register_topic(MODEL_EXHAUST_SV, PUBSUB_TYPE_BOOLEAN, false);
    pubsub_register_topic(MODEL_HEATER_SV, PUBSUB_TYPE_BOOLEAN, false);
    pubsub_register_topic(MODEL_LIGHT_SV, PUBSUB_TYPE_BOOLEAN, false);
    pubsub_register_topic(MODEL_RECIRC_SV, PUBSUB_TYPE_BOOLEAN, false);

    pubsub_register_topic(MODEL_TEMP_PV, PUBSUB_TYPE_DOUBLE, true);
    pubsub_register_topic(MODEL_HUM_PV, PUBSUB_TYPE_DOUBLE, true);
    pubsub_register_topic(MODEL_CO2_PV, PUBSUB_TYPE_DOUBLE, true);

    pubsub_register_topic(MODEL_TEMP_SV_NIGHT, PUBSUB_TYPE_DOUBLE, false);
    pubsub_register_topic(MODEL_HUM_SV_NIGHT, PUBSUB_TYPE_DOUBLE, false);
    pubsub_register_topic(MODEL_CO2_SV_NIGHT, PUBSUB_TYPE_DOUBLE, false);

    pubsub_register_topic(MODEL_CURRENT_TIME, PUBSUB_TYPE_INT, true);
    pubsub_register_topic(MODEL_BEGIN_OF_DAY, PUBSUB_TYPE_INT, false);
    pubsub_register_topic(MODEL_BEGIN_OF_NIGHT, PUBSUB_TYPE_INT, false);

    pubsub_register_topic(MODEL_TEMP_SV, PUBSUB_TYPE_DOUBLE, false);
    pubsub_register_topic(MODEL_HUM_SV, PUBSUB_TYPE_DOUBLE, false);
    pubsub_register_topic(MODEL_CO2_SV, PUBSUB_TYPE_DOUBLE, false);
}
