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

void hmi_settings_set_clock(time_t timestamp);

#ifdef __cplusplus
}
#endif

#endif /* _HMI_SETTINGS_H_ */
