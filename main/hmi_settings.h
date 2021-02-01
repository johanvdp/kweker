// The author disclaims copyright to this source code.
#ifndef _HMI_SETTINGS_H_
#define _HMI_SETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/*
 * -------------
 * Tab: Settings
 * -------------
 */
lv_obj_t* hmi_settings_create_tab(lv_obj_t *parent);

/** set current time hour */
extern lv_obj_t *hmi_spinbox_time_hour;
/** set current time minute */
extern lv_obj_t *hmi_spinbox_time_minute;
/** set begin of day time hour */
extern lv_obj_t *hmi_spinbox_begin_of_day_hour;
/** set begin of day time minute */
extern lv_obj_t *hmi_spinbox_begin_of_day_minute;
/** set begin of night time hour */
extern lv_obj_t *hmi_spinbox_begin_of_night_hour;
/** set begin of night time minute */
extern lv_obj_t *hmi_spinbox_begin_of_night_minute;
/** set day temperature */
extern lv_obj_t *hmi_spinbox_day_temperature;
/** set day humidity */
extern lv_obj_t *hmi_spinbox_day_humidity;
/** set day CO2 concentration */
extern lv_obj_t *hmi_spinbox_day_co2;
/** set night temperature */
extern lv_obj_t *hmi_spinbox_night_temperature;
/** set night humidity */
extern lv_obj_t *hmi_spinbox_night_humidity;
/** set night CO2 concentration */
extern lv_obj_t *hmi_spinbox_night_co2;

#ifdef __cplusplus
}
#endif

#endif /* _HMI_SETTINGS_H_ */
