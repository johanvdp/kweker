// The author disclaims copyright to this source code.

#ifndef _HMI_H_
#define _HMI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "lvgl.h"

#include "model.h"

#define HMI_MARGIN 5

/*
 * -------
 * Toolbar
 * -------
 */

typedef void (*hmi_bool_callback_t)(bool value);
typedef void (*hmi_int_callback_t)(uint64_t value);
typedef void (*hmi_time_callback_t)(time_t time);
typedef void (*hmi_double_callback_t)(double value);

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
void hmi_set_circadian(bool day);

void hmi_set_lamp(bool active);
void hmi_set_exhaust(bool active);
void hmi_set_heater(bool active);
void hmi_set_recirc(bool active);

#ifdef __cplusplus
}
#endif

#endif /* _HMI_H_ */
