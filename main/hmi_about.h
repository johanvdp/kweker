// The author disclaims copyright to this source code.
#ifndef _HMI_ABOUT_H_
#define _HMI_ABOUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/*
 * ----------
 * Tab: About
 * ----------
 */
lv_obj_t* hmi_about_create_tab(lv_obj_t *parent);

/** about page content */
extern lv_obj_t *hmi_label_about;

#ifdef __cplusplus
}
#endif

#endif /* _HMI_ABOUT_H_ */
