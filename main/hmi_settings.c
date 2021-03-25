// The author disclaims copyright to this source code.

#include "hmi_settings.h"
#include "hmi_timespinner.h"
#include "hmi_numberspinner.h"

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
/**
 * set day temperature
 * format: 00.0
 */
static hmi_numberspinner_t hmi_numberspinner_day_temperature;
static char hmi_numberspinner_day_temperature_representation[] =
        { 0, 0, 0, 0, 0 };
static char *hmi_numberspinner_day_temperature_representation_format = "%.1lf";
/**
 * set day humidity
 * format: 000
 */
static hmi_numberspinner_t hmi_numberspinner_day_humidity;
static char hmi_numberspinner_day_humidity_representation[] = { 0, 0, 0, 0, 0 };
static char *hmi_numberspinner_day_humidity_representation_format = "%.0lf";
/**
 * set day CO2 concentration
 * format: 0000
 */
static hmi_numberspinner_t hmi_numberspinner_day_co2;
static char hmi_numberspinner_day_co2_representation[] = { 0, 0, 0, 0, 0 };
static char *hmi_numberspinner_day_co2_representation_format = "%.0lf";
/**
 * set night temperature
 * format: 00.0
 */
static hmi_numberspinner_t hmi_numberspinner_night_temperature;
static char hmi_numberspinner_night_temperature_representation[] = { 0, 0, 0, 0,
        0 };
static char *hmi_numberspinner_night_temperature_representation_format = "%.1lf";
/**
 * set night humidity
 * format: 000
 */
static hmi_numberspinner_t hmi_numberspinner_night_humidity;
static char hmi_numberspinner_night_humidity_representation[] =
        { 0, 0, 0, 0, 0 };
static char *hmi_numberspinner_night_humidity_representation_format = "%.0lf";
/**
 * set night CO2 concentration
 * format: 0000
 */
static hmi_numberspinner_t hmi_numberspinner_night_co2;
static char hmi_numberspinner_night_co2_representation[] = { 0, 0, 0, 0, 0 };
static char *hmi_numberspinner_night_co2_representation_format = "%.0lf";

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
    hmi_numberspinner_create(tab,
    HMI_VALUE_X, 3 * HMI_SETTING_HEIGHT, HMI_SETTING_WIDTH, 0.0, 50.0, 0.5,
            hmi_numberspinner_day_temperature_representation,
            sizeof hmi_numberspinner_day_temperature_representation,
            hmi_numberspinner_day_temperature_representation_format,
            &hmi_numberspinner_day_temperature);

    /** set day humidity */
    hmi_settings_create_label(tab, HMI_MARGIN, 4 * HMI_SETTING_HEIGHT,
            "Day humidity:");
    hmi_numberspinner_create(tab,
    HMI_VALUE_X, 4 * HMI_SETTING_HEIGHT, HMI_SETTING_WIDTH, 0.0, 100.0, 1.0,
            hmi_numberspinner_day_humidity_representation,
            sizeof hmi_numberspinner_day_humidity_representation,
            hmi_numberspinner_day_humidity_representation_format,
            &hmi_numberspinner_day_humidity);

    /** set day CO2 concentration */
    hmi_settings_create_label(tab, HMI_MARGIN, 5 * HMI_SETTING_HEIGHT,
            "Day CO2 concentration:");
    hmi_numberspinner_create(tab, HMI_VALUE_X, 5 * HMI_SETTING_HEIGHT,
    HMI_SETTING_WIDTH, 0.0, 10000.0, 100.0,
            hmi_numberspinner_day_co2_representation,
            sizeof hmi_numberspinner_day_co2_representation,
            hmi_numberspinner_day_co2_representation_format,
            &hmi_numberspinner_day_co2);

    /** set night temperature */
    hmi_settings_create_label(tab, HMI_MARGIN, 6 * HMI_SETTING_HEIGHT,
            "Night temperature:");
    hmi_numberspinner_create(tab,
    HMI_VALUE_X, 6 * HMI_SETTING_HEIGHT, HMI_SETTING_WIDTH, 0.0, 50.0, 0.5,
            hmi_numberspinner_night_temperature_representation,
            sizeof hmi_numberspinner_night_temperature_representation,
            hmi_numberspinner_night_temperature_representation_format,
            &hmi_numberspinner_night_temperature);

    /** set night humidity */
    hmi_settings_create_label(tab, HMI_MARGIN, 7 * HMI_SETTING_HEIGHT,
            "Night humidity:");
    hmi_numberspinner_create(tab,
    HMI_VALUE_X, 7 * HMI_SETTING_HEIGHT, HMI_SETTING_WIDTH, 0.0, 100.0, 1.0,
            hmi_numberspinner_night_humidity_representation,
            sizeof hmi_numberspinner_night_humidity_representation,
            hmi_numberspinner_night_humidity_representation_format,
            &hmi_numberspinner_night_humidity);

    /** set night CO2 concentration */
    hmi_settings_create_label(tab, HMI_MARGIN, 8 * HMI_SETTING_HEIGHT,
            "Night CO2 concentration:");
    hmi_numberspinner_create(tab, HMI_VALUE_X, 8 * HMI_SETTING_HEIGHT,
    HMI_SETTING_WIDTH, 0.0, 10000.0, 100.0,
            hmi_numberspinner_night_co2_representation,
            sizeof hmi_numberspinner_night_co2_representation,
            hmi_numberspinner_night_co2_representation_format,
            &hmi_numberspinner_night_co2);

    hmi_settings_create_label(tab, HMI_MARGIN, 9 * HMI_SETTING_HEIGHT,
            "Color theme dark:");
    hmi_settings_create_button_theme(tab, HMI_VALUE_X, 9 * HMI_SETTING_HEIGHT);

    return tab;
}

void hmi_settings_set_current_time(time_t timestamp)
{
    hmi_timespinner_set_time(&hmi_timespinner_current_time, timestamp);
}

void hmi_settings_set_current_time_callback(hmi_time_callback_t callback)
{
    hmi_timespinner_current_time.callback = callback;
}

void hmi_settings_set_begin_of_day(time_t timestamp)
{
    hmi_timespinner_set_time(&hmi_timespinner_begin_of_day, timestamp);
}

void hmi_settings_set_begin_of_day_callback(hmi_time_callback_t callback)
{
    hmi_timespinner_begin_of_day.callback = callback;
}

void hmi_settings_set_begin_of_night(time_t timestamp)
{
    hmi_timespinner_set_time(&hmi_timespinner_begin_of_night, timestamp);
}

void hmi_settings_set_begin_of_night_callback(hmi_time_callback_t callback)
{
    hmi_timespinner_begin_of_night.callback = callback;
}

void hmi_settings_set_temp_day(double value)
{
    hmi_numberspinner_set_value(&hmi_numberspinner_day_temperature, value);
}

void hmi_settings_set_temp_day_callback(hmi_double_callback_t callback)
{
    hmi_numberspinner_day_temperature.callback = callback;
}

void hmi_settings_set_temp_night(double value)
{
    hmi_numberspinner_set_value(&hmi_numberspinner_night_temperature, value);
}

void hmi_settings_set_temp_night_callback(hmi_double_callback_t callback)
{
    hmi_numberspinner_night_temperature.callback = callback;
}

void hmi_settings_set_hum_day(double value)
{
    hmi_numberspinner_set_value(&hmi_numberspinner_day_humidity, value);
}

void hmi_settings_set_hum_day_callback(hmi_double_callback_t callback)
{
    hmi_numberspinner_day_humidity.callback = callback;
}

void hmi_settings_set_hum_night(double value)
{
    hmi_numberspinner_set_value(&hmi_numberspinner_night_humidity, value);
}

void hmi_settings_set_hum_night_callback(hmi_double_callback_t callback)
{
    hmi_numberspinner_night_humidity.callback = callback;
}

void hmi_settings_set_co2_day(double value)
{
    hmi_numberspinner_set_value(&hmi_numberspinner_day_co2, value);
}

void hmi_settings_set_co2_day_callback(hmi_double_callback_t callback)
{
    hmi_numberspinner_day_co2.callback = callback;
}

void hmi_settings_set_co2_night(double value)
{
    hmi_numberspinner_set_value(&hmi_numberspinner_night_co2, value);
}

void hmi_settings_set_co2_night_callback(hmi_double_callback_t callback)
{
    hmi_numberspinner_night_co2.callback = callback;
}
