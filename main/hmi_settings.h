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

typedef void (*hmi_settings_time_callback_t)(time_t time);
typedef void (*hmi_settings_double_callback_t)(double value);

lv_obj_t* hmi_settings_create_tab(lv_obj_t *parent);

void hmi_settings_set_time(time_t timestamp);
void hmi_settings_set_time_callback(hmi_settings_time_callback_t callback);

void hmi_settings_set_day_callback(hmi_settings_time_callback_t callback);
void hmi_settings_set_night_callback(hmi_settings_time_callback_t callback);

void hmi_settings_set_temp_day_callback(hmi_settings_double_callback_t callback);
void hmi_settings_set_temp_night_callback(hmi_settings_double_callback_t callback);

void hmi_settings_set_hum_day_callback(hmi_settings_double_callback_t callback);
void hmi_settings_set_hum_night_callback(hmi_settings_double_callback_t callback);

void hmi_settings_set_co2_day_callback(hmi_settings_double_callback_t callback);
void hmi_settings_set_co2_night_callback(hmi_settings_double_callback_t callback);


#ifdef __cplusplus
}
#endif

#endif /* _HMI_SETTINGS_H_ */
