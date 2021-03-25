// The author disclaims copyright to this source code.

#ifndef _HMI_NUMBERSPINNER_H_
#define _HMI_NUMBERSPINNER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "hmi.h"

typedef struct
{
    lv_obj_t *textarea;
    double value;
    double min;
    double max;
    double granularity;
    char *representation;
    size_t representation_size;
    char *representation_format;
    hmi_double_callback_t callback;
} hmi_numberspinner_t;

void hmi_numberspinner_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
        lv_coord_t width, double min, double max, double granularity,
        char *representation, size_t representation_size,
        char *representation_format, hmi_numberspinner_t *spinner);

void hmi_numberspinner_set_value(hmi_numberspinner_t *spinner, double value);

#ifdef __cplusplus
}
#endif

#endif /* _HMI_NUMBERSPINNER_H_ */
