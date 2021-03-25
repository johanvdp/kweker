// The author disclaims copyright to this source code.

#include "hmi_numberspinner.h"

static const char *TAG = "hmi_numberspinner";

static void hmi_numberspinner_increment_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        hmi_numberspinner_t *spinner = (hmi_numberspinner_t*) btn->user_data;
        if (spinner->callback != NULL) {
            // to next granularity
            double value = spinner->value;
            uint32_t multiples = (int) (value / spinner->granularity);
            double rounded = spinner->granularity * multiples;
            double next = rounded + spinner->granularity;
            // limit to boundary
            if (next > spinner->max) {
                next = spinner->max;
            }
            spinner->callback(next);
        }
    }
}

static void hmi_numberspinner_decrement_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        hmi_numberspinner_t *spinner = (hmi_numberspinner_t*) btn->user_data;
        if (spinner->callback != NULL) {
            // to previous granularity
            double value = spinner->value;
            int multiples = (int) (value / spinner->granularity);
            double rounded = spinner->granularity * multiples;
            double prev = rounded - spinner->granularity;
            // limit to boundary
            if (prev < spinner->min) {
                prev = spinner->min;
            }
            spinner->callback(prev);
        }
    }
}

void hmi_numberspinner_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
        lv_coord_t width, double min, double max, double granularity,
        char *representation, size_t representation_size,
        char *representation_format, hmi_numberspinner_t *spinner)
{
    lv_obj_t *textarea = lv_textarea_create(parent, NULL);
    spinner->textarea = textarea;
    spinner->min = min;
    spinner->max = max;
    spinner->granularity = granularity;
    spinner->representation = representation;
    spinner->representation_size = representation_size;
    spinner->representation_format = representation_format;

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
    /** link to corresponding numberspinner for use in event handler */
    btn_plus->user_data = spinner;
    lv_obj_set_size(btn_plus, height, height);
    lv_obj_align(btn_plus, textarea, LV_ALIGN_OUT_RIGHT_MID, HMI_MARGIN, 0);
    lv_theme_apply(btn_plus, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(btn_plus, LV_BTN_PART_MAIN,
            LV_STATE_DEFAULT,
            LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(btn_plus, hmi_numberspinner_increment_event_cb);

    lv_obj_t *btn_min = lv_btn_create(parent, btn_plus);
    /** link to corresponding numberspinner for use in event handler */
    btn_min->user_data = spinner;
    lv_obj_align(btn_min, textarea, LV_ALIGN_OUT_LEFT_MID, -HMI_MARGIN, 0);
    lv_obj_set_style_local_value_str(btn_min, LV_BTN_PART_MAIN,
            LV_STATE_DEFAULT,
            LV_SYMBOL_MINUS);
    lv_obj_set_event_cb(btn_min, hmi_numberspinner_decrement_event_cb);
}

void hmi_numberspinner_set_value(hmi_numberspinner_t *spinner, double value)
{
    if (hmi_semaphore_take("hmi_numberspinner_set_value")) {

        spinner->value = value;

        snprintf(spinner->representation, spinner->representation_size,
                spinner->representation_format, value);
        lv_textarea_set_text(spinner->textarea, spinner->representation);
    }
    hmi_semaphore_give();
}
