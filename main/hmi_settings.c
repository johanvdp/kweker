// The author disclaims copyright to this source code.

#include "hmi_settings.h"
#include "hmi_timespinner.h"

#define HMI_SETTING_HEIGHT 32
#define HMI_SETTING_WIDTH 60
#define HMI_VALUE_X 360
#define HMI_CURRENT_TIME_GRANULARITY_S 60
#define HMI_NIGHT_DAY_GRANULARITY_S (15 * 60)

static const char *TAG = "hmi_settings";

/** set current time */
static hmi_timespinner_t hmi_timespinner_current_time;
/** set begin of day time */
static hmi_timespinner_t hmi_timespinner_begin_of_day;
/** set begin of night time */
static hmi_timespinner_t hmi_timespinner_begin_of_night;
/** set day temperature */
static lv_obj_t *hmi_spinbox_day_temperature;
/** set day humidity */
static lv_obj_t *hmi_spinbox_day_humidity;
/** set day CO2 concentration */
static lv_obj_t *hmi_spinbox_day_co2;
/** set night temperature */
static lv_obj_t *hmi_spinbox_night_temperature;
/** set night humidity */
static lv_obj_t *hmi_spinbox_night_humidity;
/** set night CO2 concentration */
static lv_obj_t *hmi_spinbox_night_co2;

static hmi_double_callback_t set_temp_day_callback;
static hmi_double_callback_t set_temp_night_callback;
static hmi_double_callback_t set_hum_day_callback;
static hmi_double_callback_t set_hum_night_callback;
static hmi_double_callback_t set_co2_day_callback;
static hmi_double_callback_t set_co2_night_callback;

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
        lv_coord_t y, uint8_t digit_count, uint8_t separator_position,
        int32_t range_min, int32_t range_max, uint32_t step)
{

    lv_obj_t *spinbox = lv_spinbox_create(parent, NULL);
    lv_obj_set_pos(spinbox, x, y);
    lv_obj_set_width(spinbox, HMI_SETTING_WIDTH);
    lv_textarea_set_text_align(spinbox, LV_LABEL_ALIGN_RIGHT);
    // not selectable nor blinking cursors
    lv_textarea_set_cursor_blink_time(spinbox, 0);
    lv_textarea_set_cursor_hidden(spinbox, true);
    lv_textarea_set_text_sel(spinbox, false);

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

static lv_obj_t* hmi_settings_create_spinbox_temperature(lv_obj_t *parent,
        lv_coord_t x, lv_coord_t y)
{

    return hmi_settings_create_spinbox(parent, x, y, 3, 2, 0, 500, 1);
}

static lv_obj_t* hmi_settings_create_spinbox_humidity(lv_obj_t *parent,
        lv_coord_t x, lv_coord_t y)
{

    return hmi_settings_create_spinbox(parent, x, y, 3, 2, 0, 999, 1);
}

static lv_obj_t* hmi_settings_create_spinbox_co2(lv_obj_t *parent, lv_coord_t x,
        lv_coord_t y)
{

    return hmi_settings_create_spinbox(parent, x, y, 4, 0, 0, 5000, 1);
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
    hmi_timespinner_create(tab, HMI_VALUE_X, 0 * HMI_SETTING_HEIGHT,
    HMI_SETTING_WIDTH,
    HMI_CURRENT_TIME_GRANULARITY_S, false, &hmi_timespinner_current_time);

    /** set begin of day */
    hmi_settings_create_label(tab, HMI_MARGIN, 1 * HMI_SETTING_HEIGHT,
            "Begin of day:");
    hmi_timespinner_create(tab, HMI_VALUE_X, 1 * HMI_SETTING_HEIGHT,
    HMI_SETTING_WIDTH, HMI_NIGHT_DAY_GRANULARITY_S, true,
            &hmi_timespinner_begin_of_day);

    /** set begin of night */
    hmi_settings_create_label(tab, HMI_MARGIN, 2 * HMI_SETTING_HEIGHT,
            "Begin of night:");
    hmi_timespinner_create(tab, HMI_VALUE_X, 2 * HMI_SETTING_HEIGHT,
    HMI_SETTING_WIDTH, HMI_NIGHT_DAY_GRANULARITY_S, true,
            &hmi_timespinner_begin_of_night);

    /** set day temperature */
    hmi_settings_create_label(tab, HMI_MARGIN, 3 * HMI_SETTING_HEIGHT,
            "Day temperature:");
    hmi_spinbox_day_temperature = hmi_settings_create_spinbox_temperature(tab,
    HMI_VALUE_X, 3 * HMI_SETTING_HEIGHT);
    /** set day humidity */
    hmi_settings_create_label(tab, HMI_MARGIN, 4 * HMI_SETTING_HEIGHT,
            "Day humidity:");
    hmi_spinbox_day_humidity = hmi_settings_create_spinbox_humidity(tab,
    HMI_VALUE_X, 4 * HMI_SETTING_HEIGHT);
    /** set day CO2 concentration */
    hmi_settings_create_label(tab, HMI_MARGIN, 5 * HMI_SETTING_HEIGHT,
            "Day CO2 concentration:");
    hmi_spinbox_day_co2 = hmi_settings_create_spinbox_co2(tab, HMI_VALUE_X,
            5 * HMI_SETTING_HEIGHT);
    /** set night temperature */
    hmi_settings_create_label(tab, HMI_MARGIN, 6 * HMI_SETTING_HEIGHT,
            "Night temperature:");
    hmi_spinbox_night_temperature = hmi_settings_create_spinbox_temperature(tab,
    HMI_VALUE_X, 6 * HMI_SETTING_HEIGHT);
    /** set night humidity */
    hmi_settings_create_label(tab, HMI_MARGIN, 7 * HMI_SETTING_HEIGHT,
            "Night humidity:");
    hmi_spinbox_night_humidity = hmi_settings_create_spinbox_humidity(tab,
    HMI_VALUE_X, 7 * HMI_SETTING_HEIGHT);
    /** set night CO2 concentration */
    hmi_settings_create_label(tab, HMI_MARGIN, 8 * HMI_SETTING_HEIGHT,
            "Night CO2 concentration:");
    hmi_spinbox_night_co2 = hmi_settings_create_spinbox_co2(tab, HMI_VALUE_X,
            8 * HMI_SETTING_HEIGHT);

    hmi_settings_create_label(tab, HMI_MARGIN, 9 * HMI_SETTING_HEIGHT,
            "Color theme dark:");
    hmi_settings_create_button_theme(tab, HMI_VALUE_X, 9 * HMI_SETTING_HEIGHT);

    return tab;
}

void hmi_settings_set_current_time(time_t timestamp)
{
    hmi_timespinner_set_time(&hmi_timespinner_current_time, timestamp);
}

void hmi_settings_set_begin_of_day(time_t timestamp)
{
    hmi_timespinner_set_time(&hmi_timespinner_begin_of_day, timestamp);
}

void hmi_settings_set_begin_of_night(time_t timestamp)
{
    hmi_timespinner_set_time(&hmi_timespinner_begin_of_night, timestamp);
}

void hmi_settings_set_current_time_callback(
        hmi_time_callback_t callback)
{
    hmi_timespinner_current_time.callback = callback;
}

void hmi_settings_set_begin_of_day_callback(
        hmi_time_callback_t callback)
{
    hmi_timespinner_begin_of_day.callback = callback;
}

void hmi_settings_set_begin_of_night_callback(
        hmi_time_callback_t callback)
{
    hmi_timespinner_begin_of_night.callback = callback;
}

void hmi_settings_set_temp_day_callback(hmi_double_callback_t callback)
{
    set_temp_day_callback = callback;
}

void hmi_settings_set_temp_night_callback(
        hmi_double_callback_t callback)
{
    set_temp_night_callback = callback;
}

void hmi_settings_set_hum_day_callback(hmi_double_callback_t callback)
{
    set_hum_day_callback = callback;
}

void hmi_settings_set_hum_night_callback(
        hmi_double_callback_t callback)
{
    set_hum_night_callback = callback;
}

void hmi_settings_set_co2_day_callback(hmi_double_callback_t callback)
{
    set_co2_day_callback = callback;
}

void hmi_settings_set_co2_night_callback(
        hmi_double_callback_t callback)
{
    set_co2_night_callback = callback;
}
