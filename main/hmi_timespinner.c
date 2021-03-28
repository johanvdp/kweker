// The author disclaims copyright to this source code.

#include "hmi_timespinner.h"

/** 2000-01-01T00:00:00 */
#define HMI_DATELESS_MIN 946684800
/** 2000-01-02T00:00:00 */
#define HMI_DATELESS_MAX 946771200
/** P24:00:00 */
#define HMI_DATELESS_PERIOD (HMI_DATELESS_MAX - HMI_DATELESS_MIN)

static const char *TAG = "hmi_timespinner";

static void hmi_timespinner_increment_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        hmi_timespinner_t *spinner = (hmi_timespinner_t*) btn->user_data;
        if (spinner->callback != NULL) {
            // to next granularity
            time_t time = spinner->time;
            time_t rounded = time - time % spinner->granularity;
            time_t next = rounded + spinner->granularity;
            // wrap within same day
            if (spinner->dateless && next > HMI_DATELESS_MAX) {
                next -= HMI_DATELESS_PERIOD;
            }
            spinner->callback(next);
        }
    }
}

static void hmi_timespinner_decrement_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        hmi_timespinner_t *spinner = (hmi_timespinner_t*) btn->user_data;
        if (spinner->callback != NULL) {
            // to previous granularity
            time_t time = spinner->time;
            uint32_t rounded = time - time % spinner->granularity;
            time_t prev = rounded - spinner->granularity;
            // wrap within same day
            if (spinner->dateless && prev < HMI_DATELESS_MIN) {
                prev += HMI_DATELESS_PERIOD;
            }
            spinner->callback(prev);
        }
    }
}

void hmi_timespinner_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
        lv_coord_t width, lv_coord_t height, uint32_t granularity,
        bool dateless, hmi_timespinner_t *spinner)
{
    lv_coord_t button_width = height;

    lv_obj_t *btn_min = lv_btn_create(parent, NULL);
    lv_theme_apply(btn_min, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(btn_min, LV_BTN_PART_MAIN,
            LV_STATE_DEFAULT,
            LV_SYMBOL_MINUS);
    lv_obj_set_size(btn_min, button_width, height);
    lv_obj_set_pos(btn_min, x, y);
    /** link to corresponding timespinner for use in event handler */
    btn_min->user_data = spinner;
    lv_obj_set_event_cb(btn_min, hmi_timespinner_decrement_event_cb);

    lv_obj_t *btn_plus = lv_btn_create(parent, NULL);
    lv_theme_apply(btn_plus, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(btn_plus, LV_BTN_PART_MAIN,
            LV_STATE_DEFAULT,
            LV_SYMBOL_PLUS);
    lv_obj_set_size(btn_plus, button_width, height);
    lv_obj_align(btn_plus, btn_min, LV_ALIGN_OUT_RIGHT_MID, width - 2 * button_width, 0);
    /** link to corresponding numberspinner for use in event handler */
    btn_plus->user_data = spinner;
    lv_obj_set_event_cb(btn_plus, hmi_timespinner_increment_event_cb);

    lv_obj_t *label = lv_label_create(parent, NULL);
    spinner->label = label;
    spinner->granularity = granularity;
    spinner->dateless = dateless;

    // make size fixed, text flexible
    lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
    lv_obj_set_width(label, width - 2 * button_width);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label, btn_min, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
}

void hmi_timespinner_set_time(hmi_timespinner_t *spinner, time_t timestamp)
{
    if (hmi_semaphore_take("hmi_timespinner_set_time")) {

        spinner->time = timestamp;

        struct tm brokentime;
        gmtime_r(&timestamp, &brokentime);
        // HH:MM\0
        char text[] = { 0, 0, 0, 0, 0, 0 };
        snprintf(text, sizeof text, "%02d:%02d", brokentime.tm_hour,
                brokentime.tm_min);
        lv_label_set_text(spinner->label, text);

        hmi_semaphore_give();
    }
}
