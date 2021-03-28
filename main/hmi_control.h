// The author disclaims copyright to this source code.

#ifndef _HMI_CONTROL_H_
#define _HMI_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "lvgl.h"

#include "hmi.h"

/*
 * ------------
 * Tab: Control
 * ------------
 */

typedef void (*hmi_control_mode_callback_t)(hmi_control_mode_t mode);

lv_obj_t* hmi_control_create_tab(lv_obj_t *parent);

void hmi_control_set_temp_pv(double pv);
void hmi_control_set_temp_sv(double sv);
void hmi_control_set_temp_hi(bool hi);
void hmi_control_set_temp_lo(bool lo);

void hmi_control_set_hum_pv(double pv);
void hmi_control_set_hum_sv(double sv);
void hmi_control_set_hum_hi(bool hi);
void hmi_control_set_hum_lo(bool lo);

void hmi_control_set_co2_pv(double pv);
void hmi_control_set_co2_sv(double sv);
void hmi_control_set_co2_hi(bool hi);
void hmi_control_set_co2_lo(bool lo);

void hmi_control_set_control_mode(hmi_control_mode_t mode);
void hmi_control_set_control_mode_callback(hmi_control_mode_callback_t callback);

void hmi_control_set_light_sv(bool active);
void hmi_control_set_light_sv_callback(hmi_bool_callback_t callback);
void hmi_control_set_exhaust_sv(bool active);
void hmi_control_set_exhaust_sv_callback(hmi_bool_callback_t callback);
void hmi_control_set_recirc_sv(bool active);
void hmi_control_set_recirc_sv_callback(hmi_bool_callback_t callback);
void hmi_control_set_heater_sv(bool active);
void hmi_control_set_heater_sv_callback(hmi_bool_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif /* _HMI_CONTROL_H_ */
