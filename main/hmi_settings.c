// The author disclaims copyright to this source code.
#include "hmi_settings.h"

#define HMI_MARGIN 5
#define HMI_SETTING_HEIGHT 32
#define HMI_SETTING_WIDTH 60
#define HMI_COLUMN1 200
#define HMI_COLUMN2 360
#define HMI_COLUMN_TIME_SEPARATOR 310

static const char *TAG = "hmi_settings";

/** set current time hour */
lv_obj_t *hmi_spinbox_time_hour;
/** set current time minute */
lv_obj_t *hmi_spinbox_time_minute;
/** set begin of day time hour */
lv_obj_t *hmi_spinbox_begin_of_day_hour;
/** set begin of day time minute */
lv_obj_t *hmi_spinbox_begin_of_day_minute;
/** set begin of night time hour */
lv_obj_t *hmi_spinbox_begin_of_night_hour;
/** set begin of night time minute */
lv_obj_t *hmi_spinbox_begin_of_night_minute;
/** set day temperature */
lv_obj_t *hmi_spinbox_day_temperature;
/** set day humidity */
lv_obj_t *hmi_spinbox_day_humidity;
/** set day CO2 concentration */
lv_obj_t *hmi_spinbox_day_co2;
/** set night temperature */
lv_obj_t *hmi_spinbox_night_temperature;
/** set night humidity */
lv_obj_t *hmi_spinbox_night_humidity;
/** set night CO2 concentration */
lv_obj_t *hmi_spinbox_night_co2;

static void hmi_settings_button_theme_cb(lv_obj_t *button, lv_event_t e)
{

    if (e == LV_EVENT_VALUE_CHANGED) {
        uint32_t flag = LV_THEME_MATERIAL_FLAG_LIGHT;
        if (lv_btn_get_state(button) == LV_BTN_STATE_CHECKED_RELEASED) flag =
                LV_THEME_MATERIAL_FLAG_DARK;

        LV_THEME_DEFAULT_INIT(lv_theme_get_color_primary(),
                lv_theme_get_color_secondary(), flag, lv_theme_get_font_small(),
                lv_theme_get_font_normal(), lv_theme_get_font_subtitle(),
                lv_theme_get_font_title());
    }
}

static lv_obj_t* hmi_settings_create_button_theme(lv_obj_t *parent,
        lv_coord_t x, lv_coord_t y)
{

    lv_obj_t *button = lv_btn_create(parent, NULL);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_size(button, HMI_SETTING_WIDTH, HMI_SETTING_HEIGHT);
    lv_btn_set_checkable(button, true);

    // follow current setting
    if (lv_theme_get_flags() & LV_THEME_MATERIAL_FLAG_DARK) {
        lv_btn_set_state(button, LV_BTN_STATE_CHECKED_RELEASED);
    } else {
        lv_btn_set_state(button, LV_BTN_STATE_RELEASED);
    }

    lv_obj_set_event_cb(button, hmi_settings_button_theme_cb);

    return button;
}

static void hmi_settings_spinbox_increment_event_cb(lv_obj_t *btn, lv_event_t e)
{

    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_obj_t *spinbox = (lv_obj_t*) btn->user_data;
        lv_spinbox_increment(spinbox);
    }
}

static void hmi_settings_spinbox_decrement_event_cb(lv_obj_t *btn, lv_event_t e)
{

    if (e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_obj_t *spinbox = (lv_obj_t*) btn->user_data;
        lv_spinbox_decrement(spinbox);
    }
}

static lv_obj_t* hmi_settings_create_spinbox(lv_obj_t *parent, lv_coord_t x,
        lv_coord_t y, lv_coord_t w, uint8_t digit_count,
        uint8_t separator_position, int32_t range_min, int32_t range_max,
        uint32_t step)
{

    lv_obj_t *spinbox = lv_spinbox_create(parent, NULL);
    lv_obj_set_pos(spinbox, x, y);
    lv_obj_set_width(spinbox, w);
    lv_textarea_set_text_align(spinbox, LV_LABEL_ALIGN_RIGHT);
    // avoid sea of blinking cursors
    lv_textarea_set_cursor_blink_time(spinbox, 0);
    lv_spinbox_set_digit_format(spinbox, digit_count, separator_position);
    lv_spinbox_set_range(spinbox, range_min, range_max);
    lv_spinbox_set_step(spinbox, step);
    lv_spinbox_set_rollover(spinbox, true);

    lv_coord_t height = lv_obj_get_height(spinbox);

    lv_obj_t *btn_plus = lv_btn_create(parent, NULL);
    /** link to corresponding spinbox for use in event handler */
    btn_plus->user_data = spinbox;
    lv_obj_set_size(btn_plus, height, height);
    lv_obj_align(btn_plus, spinbox, LV_ALIGN_OUT_RIGHT_MID, HMI_MARGIN, 0);
    lv_theme_apply(btn_plus, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(btn_plus, LV_BTN_PART_MAIN,
            LV_STATE_DEFAULT,
            LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(btn_plus, hmi_settings_spinbox_increment_event_cb);

    lv_obj_t *btn_min = lv_btn_create(parent, btn_plus);
    /** link to corresponding spinbox for use in event handler */
    btn_min->user_data = spinbox;
    lv_obj_align(btn_min, spinbox, LV_ALIGN_OUT_LEFT_MID, -HMI_MARGIN, 0);
    lv_obj_set_style_local_value_str(btn_min, LV_BTN_PART_MAIN,
            LV_STATE_DEFAULT,
            LV_SYMBOL_MINUS);
    lv_obj_set_event_cb(btn_min, hmi_settings_spinbox_decrement_event_cb);

    return spinbox;
}

static lv_obj_t* hmi_settings_create_spinbox_hour(lv_obj_t *parent,
        lv_coord_t x, lv_coord_t y)
{

    return hmi_settings_create_spinbox(parent, x, y, HMI_SETTING_WIDTH, 2, 0, 0,
            23, 1);
}

static lv_obj_t* hmi_settings_create_spinbox_minute(lv_obj_t *parent,
        lv_coord_t x, lv_coord_t y)
{

    return hmi_settings_create_spinbox(parent, x, y, HMI_SETTING_WIDTH, 2, 0, 0,
            59, 1);
}

static lv_obj_t* hmi_settings_create_spinbox_temperature(lv_obj_t *parent,
        lv_coord_t x, lv_coord_t y)
{

    return hmi_settings_create_spinbox(parent, x, y, HMI_SETTING_WIDTH, 3, 2, 0,
            500, 1);
}

static lv_obj_t* hmi_settings_create_spinbox_humidity(lv_obj_t *parent,
        lv_coord_t x, lv_coord_t y)
{

    return hmi_settings_create_spinbox(parent, x, y, HMI_SETTING_WIDTH, 3, 2, 0,
            999, 1);
}

static lv_obj_t* hmi_settings_create_spinbox_co2(lv_obj_t *parent, lv_coord_t x,
        lv_coord_t y)
{

    return hmi_settings_create_spinbox(parent, x, y, HMI_SETTING_WIDTH, 4, 0, 0,
            5000, 1);
}

static lv_obj_t* hmi_settings_create_label(lv_obj_t *parent, lv_coord_t x,
        lv_coord_t y, const char *text)
{

    lv_obj_t *label = lv_label_create(parent, NULL);
    lv_obj_set_pos(label, x, y + HMI_MARGIN);
    lv_label_set_text(label, text);

    return label;
}

lv_obj_t* hmi_settings_create_tab(lv_obj_t *parent)
{

    lv_obj_t *tab = lv_tabview_add_tab(parent, "Settings");

    /** set current time */
    hmi_settings_create_label(tab, HMI_MARGIN, 0 * HMI_SETTING_HEIGHT,
            "Current time:");
    hmi_spinbox_time_hour = hmi_settings_create_spinbox_hour(tab, HMI_COLUMN1,
            0 * HMI_SETTING_HEIGHT);
    hmi_spinbox_time_minute = hmi_settings_create_spinbox_minute(tab,
    HMI_COLUMN2, 0 * HMI_SETTING_HEIGHT);
    hmi_settings_create_label(tab, HMI_COLUMN_TIME_SEPARATOR,
            0 * HMI_SETTING_HEIGHT, ":");

    /** set begin of day time */
    hmi_settings_create_label(tab, HMI_MARGIN, 1 * HMI_SETTING_HEIGHT,
            "Begin of day time:");
    hmi_spinbox_begin_of_day_hour = hmi_settings_create_spinbox_hour(tab,
    HMI_COLUMN1, 1 * HMI_SETTING_HEIGHT);
    hmi_spinbox_begin_of_day_minute = hmi_settings_create_spinbox_minute(tab,
    HMI_COLUMN2, 1 * HMI_SETTING_HEIGHT);
    hmi_settings_create_label(tab, HMI_COLUMN_TIME_SEPARATOR,
            1 * HMI_SETTING_HEIGHT, ":");

    /** set begin of night time */
    hmi_settings_create_label(tab, HMI_MARGIN, 2 * HMI_SETTING_HEIGHT,
            "Begin of night time:");
    hmi_spinbox_begin_of_night_hour = hmi_settings_create_spinbox_hour(tab,
    HMI_COLUMN1, 2 * HMI_SETTING_HEIGHT);
    hmi_spinbox_begin_of_night_minute = hmi_settings_create_spinbox_minute(tab,
    HMI_COLUMN2, 2 * HMI_SETTING_HEIGHT);
    hmi_settings_create_label(tab, HMI_COLUMN_TIME_SEPARATOR,
            2 * HMI_SETTING_HEIGHT, ":");

    /** set day temperature */
    hmi_settings_create_label(tab, HMI_MARGIN, 3 * HMI_SETTING_HEIGHT,
            "Day temperature:");
    hmi_spinbox_day_temperature = hmi_settings_create_spinbox_temperature(tab,
    HMI_COLUMN2, 3 * HMI_SETTING_HEIGHT);
    /** set day humidity */
    hmi_settings_create_label(tab, HMI_MARGIN, 4 * HMI_SETTING_HEIGHT,
            "Day humidity:");
    hmi_spinbox_day_humidity = hmi_settings_create_spinbox_humidity(tab,
    HMI_COLUMN2, 4 * HMI_SETTING_HEIGHT);
    /** set day CO2 concentration */
    hmi_settings_create_label(tab, HMI_MARGIN, 5 * HMI_SETTING_HEIGHT,
            "Day CO2 concentration:");
    hmi_spinbox_day_co2 = hmi_settings_create_spinbox_co2(tab, HMI_COLUMN2,
            5 * HMI_SETTING_HEIGHT);
    /** set night temperature */
    hmi_settings_create_label(tab, HMI_MARGIN, 6 * HMI_SETTING_HEIGHT,
            "Night temperature:");
    hmi_spinbox_night_temperature = hmi_settings_create_spinbox_temperature(tab,
    HMI_COLUMN2, 6 * HMI_SETTING_HEIGHT);
    /** set night humidity */
    hmi_settings_create_label(tab, HMI_MARGIN, 7 * HMI_SETTING_HEIGHT,
            "Night humidity:");
    hmi_spinbox_night_humidity = hmi_settings_create_spinbox_humidity(tab,
    HMI_COLUMN2, 7 * HMI_SETTING_HEIGHT);
    /** set night CO2 concentration */
    hmi_settings_create_label(tab, HMI_MARGIN, 8 * HMI_SETTING_HEIGHT,
            "Night CO2 concentration:");
    hmi_spinbox_night_co2 = hmi_settings_create_spinbox_co2(tab, HMI_COLUMN2,
            8 * HMI_SETTING_HEIGHT);

    hmi_settings_create_label(tab, HMI_MARGIN, 9 * HMI_SETTING_HEIGHT,
            "Color theme dark:");
    hmi_settings_create_button_theme(tab, HMI_COLUMN2, 9 * HMI_SETTING_HEIGHT);

    return tab;
}

void hmi_settings_set_clock(time_t timestamp)
{
    if (hmi_semaphore_take("hmi_settings_set_clock")) {

        struct tm brokentime;
        gmtime_r(&timestamp, &brokentime);

        lv_spinbox_set_value(hmi_spinbox_time_hour, brokentime.tm_hour);
        lv_spinbox_set_value(hmi_spinbox_time_minute, brokentime.tm_min);

        hmi_semaphore_give();
    }
}
