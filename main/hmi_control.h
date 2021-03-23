// The author disclaims copyright to this source code.
#ifndef _HMI_CONTROL_H_
#define _HMI_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "hmi.h"

/*
 * ------------
 * Tab: Control
 * ------------
 */

typedef struct
{
    lv_obj_t *bar;
    double bar_bias;
    double bar_gain;
    lv_obj_t *label_sv;
    lv_obj_t *label_pv;
    lv_obj_t *label_lo;
    lv_obj_t *label_hi;
} hmi_control_t;

typedef enum
{
    HMI_CONTROL_MODE_OFF = 0,
    HMI_CONTROL_MODE_MANUAL = 1,
    HMI_CONTROL_MODE_AUTO = 2
} hmi_control_mode_t;

typedef void (*hmi_control_mode_callback_t)(hmi_control_mode_t mode);


/** temperature control */
extern hmi_control_t hmi_control_temperature;
/** humidity control */
extern hmi_control_t hmi_control_humidity;
/** CO2 concentration control */
extern hmi_control_t hmi_control_co2;

lv_obj_t* hmi_control_create_tab(lv_obj_t *parent);

void hmi_control_set_pv(hmi_control_t *target, double pv);
void hmi_control_set_sv(hmi_control_t *target, double sv);
void hmi_control_set_hi(hmi_control_t *target, bool hi);
void hmi_control_set_lo(hmi_control_t *target, bool lo);
void hmi_control_set_control_mode(hmi_control_mode_t mode);
void hmi_control_set_control_mode_callback(hmi_control_mode_callback_t *callback);


#ifdef __cplusplus
}
#endif

#endif /* _HMI_CONTROL_H_ */
