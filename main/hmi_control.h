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

/** temperature control */
extern hmi_control_t hmi_control_temperature;
/** humidity control */
extern hmi_control_t hmi_control_humidity;
/** CO2 concentration control */
extern hmi_control_t hmi_control_co2;

/** selector control mode */
extern lv_obj_t *hmi_control_mode_btnmatrix;
/** setpoint manual control */
extern lv_obj_t *hmi_control_manual_btnmatrix;

lv_obj_t* hmi_control_create_tab(lv_obj_t *parent);

void hmi_control_set_pv(hmi_control_t *target, double pv);
void hmi_control_set_sv(hmi_control_t *target, double sv);
void hmi_control_set_hi(hmi_control_t *target, bool hi);
void hmi_control_set_lo(hmi_control_t *target, bool lo);
void hmi_control_set_control_off();
void hmi_control_set_control_manual();
void hmi_control_set_control_auto();

#ifdef __cplusplus
}
#endif

#endif /* _HMI_CONTROL_H_ */
