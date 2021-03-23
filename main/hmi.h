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

/*
 * -------
 * Toolbar
 * -------
 */

/** Synchronize all access to the hmi using one semaphore */
bool hmi_semaphore_take(const char *function_name);
void hmi_semaphore_give();

void hmi_initialize();
void hmi_set_clock(time_t time);
void hmi_set_control_mode_off();
void hmi_set_control_mode_manual();
void hmi_set_control_mode_auto();

#ifdef __cplusplus
}
#endif

#endif /* _HMI_H_ */
