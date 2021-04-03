// The author disclaims copyright to this source code.

#include "hmi_datespinner.h"

/** 2000-01-01T00:00:00 */
#define HMI_DATE_MIN 946684800
/** short press, one day in seconds */
#define HMI_DATE_GRANULARITY_SHORT (24 * 60 * 60)
/** long press, one month in seconds */
#define HMI_DATE_GRANULARITY_LONG (31 * 24 * 60 * 60)

//static const char *TAG = "hmi_datespinner";

/**
 * truncate time to something valid
 *
 * @param time time in
 * @return time out
 */
static time_t hmi_datespinner_trunc_date(time_t time)
{
    time_t wrapped_time = time;
    if (time < HMI_DATE_MIN) {
        wrapped_time = HMI_DATE_MIN;
    }
    return wrapped_time;
}

static void hmi_datespinner_increment_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        hmi_datespinner_t *spinner = (hmi_datespinner_t*) btn->user_data;
        if (spinner->callback != NULL) {
            // to next granularity
            time_t time = hmi_datespinner_trunc_date(spinner->time);
            time_t granularity = (e == LV_EVENT_LONG_PRESSED_REPEAT) ? HMI_DATE_GRANULARITY_LONG : HMI_DATE_GRANULARITY_SHORT;
            time_t next = time + granularity;
            spinner->callback(next);
        }
    }
}

static void hmi_datespinner_decrement_event_cb(lv_obj_t *btn, lv_event_t e)
{
    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        hmi_datespinner_t *spinner = (hmi_datespinner_t*) btn->user_data;
        if (spinner->callback != NULL) {
            // to previous granularity
            time_t time = hmi_datespinner_trunc_date(spinner->time);
            time_t granularity = (e == LV_EVENT_LONG_PRESSED_REPEAT) ? HMI_DATE_GRANULARITY_LONG : HMI_DATE_GRANULARITY_SHORT;
            time_t prev = time - granularity;
            spinner->callback(prev);
        }
    }
}

void hmi_datespinner_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t width, lv_coord_t height,
        hmi_datespinner_t *spinner)
{
    lv_coord_t button_width = height;

    lv_obj_t *btn_min = lv_btn_create(parent, NULL);
    lv_theme_apply(btn_min, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(btn_min, LV_BTN_PART_MAIN, LV_STATE_DEFAULT,
    LV_SYMBOL_MINUS);
    lv_obj_set_size(btn_min, button_width, height);
    lv_obj_set_pos(btn_min, x, y);
    /** link to corresponding timespinner for use in event handler */
    btn_min->user_data = spinner;
    lv_obj_set_event_cb(btn_min, hmi_datespinner_decrement_event_cb);

    lv_obj_t *btn_plus = lv_btn_create(parent, NULL);
    lv_theme_apply(btn_plus, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(btn_plus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT,
    LV_SYMBOL_PLUS);
    lv_obj_set_size(btn_plus, button_width, height);
    lv_obj_align(btn_plus, btn_min, LV_ALIGN_OUT_RIGHT_MID, width - 2 * button_width, 0);
    /** link to corresponding numberspinner for use in event handler */
    btn_plus->user_data = spinner;
    lv_obj_set_event_cb(btn_plus, hmi_datespinner_increment_event_cb);

    lv_obj_t *label = lv_label_create(parent, NULL);
    spinner->label = label;

    // make size fixed, text flexible
    lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
    lv_obj_set_width(label, width - 2 * button_width);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label, btn_min, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
}

void hmi_datespinner_set_time(hmi_datespinner_t *spinner, time_t timestamp)
{
    if (hmi_semaphore_take("hmi_datespinner_set_time")) {

        spinner->time = timestamp;

        struct tm brokentime;
        gmtime_r(&timestamp, &brokentime);
        // YYYY-MM-DD\0
        char text[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        int year= brokentime.tm_year + 1900; //< 1900-based month
        int month = brokentime.tm_mon + 1; //< 0-based month
        int result = snprintf(text, sizeof text, "%04d-%02d-%02d", year, month, brokentime.tm_mday);
        // should always fit, but need to check to avoid compiler error
        if (result < 0) {
            // truncated
        } else {
            lv_label_set_text(spinner->label, text);
        }
        hmi_semaphore_give();
    }
}
