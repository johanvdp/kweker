// The author disclaims copyright to this source code.

#ifndef _HMI_DATESPINNER_H_
#define _HMI_DATESPINNER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "hmi.h"

typedef struct
{
    lv_obj_t *label;
    time_t time;
    hmi_time_callback_t callback;
} hmi_datespinner_t;

void hmi_datespinner_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t width, lv_coord_t height,
        hmi_datespinner_t *spinner);
void hmi_datespinner_set_time(hmi_datespinner_t *spinner, time_t timestamp);

#ifdef __cplusplus
}
#endif

#endif /* _HMI_DATESPINNER_H_ */
