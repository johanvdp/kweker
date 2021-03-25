// The author disclaims copyright to this source code.

#ifndef _HMI_TIMESPINNER_H_
#define _HMI_TIMESPINNER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "hmi.h"

typedef struct
{
    lv_obj_t *textarea;
    time_t time;
    uint32_t granularity;
    bool dateless;
    hmi_settings_time_callback_t callback;
} hmi_timespinner_t;

void hmi_timespinner_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
        lv_coord_t width, uint32_t granularity, bool dateless,
        hmi_timespinner_t *spinner);
void hmi_timespinner_set_time(hmi_timespinner_t *spinner, time_t timestamp);

#ifdef __cplusplus
}
#endif

#endif /* _HMI_TIMESPINNER_H_ */
