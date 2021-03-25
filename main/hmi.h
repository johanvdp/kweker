// The author disclaims copyright to this source code.

#ifndef _HMI_H_
#define _HMI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "lvgl.h"

/*
 * -------
 * Toolbar
 * -------
 */

typedef enum
{
    HMI_CONTROL_MODE_OFF = 0,
    HMI_CONTROL_MODE_MANUAL = 1,
    HMI_CONTROL_MODE_AUTO = 2
} hmi_control_mode_t;

/** Synchronize all access to the hmi using one semaphore */
bool hmi_semaphore_take(const char *function_name);
void hmi_semaphore_give();

void hmi_initialize();
void hmi_set_current_time(time_t time);
void hmi_set_control_mode(hmi_control_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* _HMI_H_ */
