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

void hmi_timespinner_create(lv_obj_t *parent, lv_coord_t x,
        lv_coord_t y, lv_coord_t width, hmi_timespinner_t *spinner)
{
    lv_obj_t *textarea = lv_textarea_create(parent, NULL);
    spinner->textarea = textarea;

    lv_obj_set_click(lv_page_get_scrollable(textarea), false);
    lv_theme_apply(textarea, LV_THEME_SPINBOX);
    // not selectable nor blinking cursors
    lv_textarea_set_one_line(textarea, true);
    lv_textarea_set_text_sel(textarea, false);
    lv_textarea_set_cursor_blink_time(textarea, 0);
    lv_textarea_set_cursor_hidden(textarea, true);

    lv_obj_set_pos(textarea, x, y);
    lv_obj_set_width(textarea, width);
    lv_textarea_set_text_align(textarea, LV_LABEL_ALIGN_RIGHT);

    lv_coord_t height = lv_obj_get_height(textarea);

    lv_obj_t *btn_plus = lv_btn_create(parent, NULL);
    /** link to corresponding timespinner for use in event handler */
    btn_plus->user_data = spinner;
    lv_obj_set_size(btn_plus, height, height);
    lv_obj_align(btn_plus, textarea, LV_ALIGN_OUT_RIGHT_MID, HMI_MARGIN, 0);
    lv_theme_apply(btn_plus, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(btn_plus, LV_BTN_PART_MAIN,
            LV_STATE_DEFAULT,
            LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(btn_plus, hmi_timespinner_increment_event_cb);

    lv_obj_t *btn_min = lv_btn_create(parent, btn_plus);
    /** link to corresponding timespinner for use in event handler */
    btn_min->user_data = spinner;
    lv_obj_align(btn_min, textarea, LV_ALIGN_OUT_LEFT_MID, -HMI_MARGIN, 0);
    lv_obj_set_style_local_value_str(btn_min, LV_BTN_PART_MAIN,
            LV_STATE_DEFAULT,
            LV_SYMBOL_MINUS);
    lv_obj_set_event_cb(btn_min, hmi_timespinner_decrement_event_cb);
}
