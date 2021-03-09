// The author disclaims copyright to this source code.
#ifndef _HMI_H_
#define _HMI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "time.h"

void hmi_initialize();
void hmi_set_clock(time_t time);

/*
 * -------
 * Toolbar
 * -------
 */

/** Synchronize all access to LVGL using this one semaphore */
extern SemaphoreHandle_t hmi_semaphore;

/** current time [24h] */
extern lv_obj_t *hmi_label_clock;
/** current circadian [day, night] */
extern lv_obj_t *hmi_label_circadian;
/** current control mode [off, manual, automatic] */
extern lv_obj_t *hmi_label_control_mode;
/** current light switch state [off, on] */
extern lv_obj_t *hmi_led_light_switch;
/** current heater switch state [off, on] */
extern lv_obj_t *hmi_led_heater_switch;
/** current exhaust fan switch state [off, on] */
extern lv_obj_t *hmi_led_exhaust_switch;
/** current recirculation fan switch state [off, on] */
extern lv_obj_t *hmi_led_recirculation_switch;

#ifdef __cplusplus
}
#endif

#endif /* _HMI_H_ */
