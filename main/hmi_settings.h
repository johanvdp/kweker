// The author disclaims copyright to this source code.

#ifndef _HMI_SETTINGS_H_
#define _HMI_SETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#include "hmi.h"

/*
 * -------------
 * Tab: Settings
 * -------------
 */

lv_obj_t* hmi_settings_create_tab(lv_obj_t *parent);

void hmi_settings_set_current_time(time_t timestamp);
void hmi_settings_set_current_time_callback(hmi_time_callback_t callback);

void hmi_settings_set_begin_of_day(time_t timestamp);
void hmi_settings_set_begin_of_day_callback(hmi_time_callback_t callback);

void hmi_settings_set_begin_of_night(time_t timestamp);
void hmi_settings_set_begin_of_night_callback(hmi_time_callback_t callback);

void hmi_settings_set_temp_day(double value);
void hmi_settings_set_temp_day_callback(hmi_double_callback_t callback);

void hmi_settings_set_temp_night(double value);
void hmi_settings_set_temp_night_callback(hmi_double_callback_t callback);

void hmi_settings_set_hum_day(double value);
void hmi_settings_set_hum_day_callback(hmi_double_callback_t callback);

void hmi_settings_set_hum_night(double value);
void hmi_settings_set_hum_night_callback(hmi_double_callback_t callback);

void hmi_settings_set_co2_day(double value);
void hmi_settings_set_co2_day_callback(hmi_double_callback_t callback);

void hmi_settings_set_co2_night(double value);
void hmi_settings_set_co2_night_callback(hmi_double_callback_t callback);


#ifdef __cplusplus
}
#endif

#endif /* _HMI_SETTINGS_H_ */
