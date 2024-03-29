// The author disclaims copyright to this source code.

#include "esp_log.h"

#include "hmi_control.h"

#define HMI_CONTROL_W 110
#define HMI_CONTROL_H 270
#define HMI_MARGIN 5

static const char *TAG = "hmi_control";

typedef struct
{
    lv_obj_t *bar;
    double bar_bias;
    double bar_gain;
    char *format_sv;
    char *format_pv;
    char *format_minmax;
    lv_obj_t *label_sv;
    lv_obj_t *label_pv;
    lv_obj_t *label_lo;
    lv_obj_t *label_hi;
} hmi_control_t;

/** temperature control */
hmi_control_t hmi_control_temperature;
/** humidity control */
hmi_control_t hmi_control_humidity;
/** CO2 concentration control */
hmi_control_t hmi_control_co2;

/** selector control mode */
static lv_obj_t *hmi_control_mode_btnmatrix;

/** setpoint manual control */
static lv_obj_t *hmi_control_manual_btnmatrix;

static hmi_control_mode_callback_t hmi_control_mode_callback;
static hmi_bool_callback_t hmi_control_light_sv_callback;
static hmi_bool_callback_t hmi_control_heater_sv_callback;
static hmi_bool_callback_t hmi_control_exhaust_sv_callback;
static hmi_bool_callback_t hmi_control_recirc_sv_callback;

/**
 * Calculate fraction of value on control bar.
 */
static double hmi_control_bar_fraction(hmi_control_t *control, double value)
{

    double fraction = (value + control->bar_bias) * control->bar_gain;
    ESP_LOGD(TAG, "hmi_control_bar_fraction, value:%lf, bias:%lf, gain:%lf, fraction:%lf", value, control->bar_bias, control->bar_gain, fraction);
    return fraction;
}

/**
 * Calculate value on control bar.
 */
static int16_t hmi_control_bar_value(hmi_control_t *control, double value)
{

    return hmi_control_bar_fraction(control, value) * 100.0;
}

/**
 * Calculate location of value on control bar.
 */
static lv_coord_t hmi_control_get_y(hmi_control_t *control, double value)
{

    lv_obj_t *bar = control->bar;
    lv_coord_t y = lv_obj_get_y(bar);
    lv_coord_t height = lv_obj_get_height(bar);

    double fraction = hmi_control_bar_fraction(control, value);
    return y + height - (fraction * height) - 5;
}

static void hmi_control_set_pv(hmi_control_t *target, double pv)
{
    if (hmi_semaphore_take("hmi_control_set_pv")) {

        lv_obj_t *bar = target->bar;
        lv_obj_t *label_pv = target->label_pv;

        lv_coord_t bar_x = lv_obj_get_x(bar);
        lv_coord_t bar_width = lv_obj_get_width(bar);

        int16_t value = hmi_control_bar_value(target, pv);
        lv_bar_set_value(bar, value, LV_ANIM_OFF);
        lv_label_set_text_fmt(label_pv, target->format_pv, pv);
        lv_coord_t pv_y = hmi_control_get_y(target, pv);
        lv_obj_set_pos(label_pv, bar_x + bar_width + HMI_MARGIN, pv_y);

        hmi_semaphore_give();
    }
}

static void hmi_control_set_sv(hmi_control_t *target, double sv)
{
    if (hmi_semaphore_take("hmi_control_set_sv")) {

        lv_obj_t *bar = target->bar;
        lv_obj_t *label_sv = target->label_sv;
        lv_coord_t bar_x = lv_obj_get_x(bar);

        lv_label_set_text_fmt(label_sv, target->format_sv, sv);
        lv_coord_t label_width = lv_obj_get_width(label_sv);
        lv_coord_t sv_y = hmi_control_get_y(target, sv);
        lv_obj_set_pos(label_sv, bar_x - label_width - HMI_MARGIN, sv_y);

        hmi_semaphore_give();
    }
}

static void hmi_control_set_hi(hmi_control_t *target, bool hi)
{
    if (hmi_semaphore_take("hmi_control_set_hi")) {

        lv_obj_t *label_hi = target->label_hi;

        lv_obj_set_style_local_text_color(label_hi, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, hi ? LV_COLOR_RED : LV_COLOR_GRAY);

        hmi_semaphore_give();
    }
}

static void hmi_control_set_lo(hmi_control_t *target, bool lo)
{
    if (hmi_semaphore_take("hmi_control_set_lo")) {

        lv_obj_t *label_lo = target->label_lo;

        lv_obj_set_style_local_text_color(label_lo, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lo ? LV_COLOR_RED : LV_COLOR_GRAY);

        hmi_semaphore_give();
    }
}

static void hmi_control_create_control(hmi_control_t *target, lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t w,
        lv_coord_t h, const char *name, double min, double max, int decimals)
{

    lv_obj_t *control = lv_cont_create(parent, NULL);
    lv_obj_clean_style_list(control, LV_CONT_PART_MAIN);
    lv_obj_set_pos(control, x, y);
    lv_obj_set_size(control, w, h);

    lv_obj_t *label_name = lv_label_create(control, NULL);
    lv_label_set_text(label_name, name);
    lv_obj_align(label_name, control, LV_ALIGN_IN_TOP_MID, 0, HMI_MARGIN);

    lv_obj_t *label_hi = lv_label_create(control, NULL);
    lv_label_set_text(label_hi, "HI");
    lv_obj_set_style_local_text_color(label_hi, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_align(label_hi, label_name, LV_ALIGN_OUT_BOTTOM_MID, 0, HMI_MARGIN);
    target->label_hi = label_hi;

    lv_obj_t *label_lo = lv_label_create(control, NULL);
    lv_label_set_text(label_lo, "LO");
    lv_obj_set_style_local_text_color(label_lo, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_align(label_lo, control, LV_ALIGN_IN_BOTTOM_MID, 0, -HMI_MARGIN);
    target->label_lo = label_lo;

    lv_obj_t *bar = lv_bar_create(control, NULL);
    lv_area_t label_hi_coords;
    lv_obj_get_coords(label_hi, &label_hi_coords);
    lv_area_t label_lo_coords;
    lv_obj_get_coords(label_lo, &label_lo_coords);
    // bar width 10%
    lv_coord_t bar_width = w / 10;
    lv_obj_set_size(bar, bar_width, label_lo_coords.y1 - label_hi_coords.y2 - 10);
    // use bias and scale to convert value to [0.0, 1.0] range
    target->bar_bias = -min;
    target->bar_gain = 1.0 / (max - min);
    lv_bar_set_type(bar, LV_BAR_TYPE_NORMAL);
    lv_bar_set_range(bar, 0, 100);
    lv_obj_align(bar, label_hi, LV_ALIGN_OUT_BOTTOM_MID, 0, HMI_MARGIN);
    lv_obj_align(bar, label_lo, LV_ALIGN_OUT_TOP_MID, 0, -HMI_MARGIN);
    target->bar = bar;
    lv_coord_t bar_x = lv_obj_get_x(bar);

    lv_obj_t *label_max = lv_label_create(control, NULL);
    asprintf(&target->format_minmax, "-%%.%dlf", decimals);
    lv_label_set_text_fmt(label_max, target->format_minmax, max);
    lv_coord_t max_y = hmi_control_get_y(target, max);
    lv_obj_set_pos(label_max, bar_x + bar_width + HMI_MARGIN, max_y);

    lv_obj_t *label_min = lv_label_create(control, NULL);
    lv_coord_t min_y = hmi_control_get_y(target, min);
    lv_obj_set_pos(label_min, bar_x + bar_width + HMI_MARGIN, min_y);
    lv_label_set_text_fmt(label_min, target->format_minmax, min);

    lv_obj_t *label_pv = lv_label_create(control, NULL);
    asprintf(&target->format_pv, "=%%.%dlf", decimals);
    lv_coord_t pv_y = hmi_control_get_y(target, min);
    lv_obj_set_pos(label_pv, bar_x + bar_width + HMI_MARGIN, pv_y);
    lv_label_set_text_fmt(label_pv, target->format_pv, min);
    target->label_pv = label_pv;

    lv_obj_t *label_sv = lv_label_create(control, NULL);
    asprintf(&target->format_sv, "%%.%dlf>", decimals);
    lv_label_set_text_fmt(label_sv, target->format_sv, min);
    lv_coord_t label_width = lv_obj_get_width(label_sv);
    lv_coord_t sv_y = hmi_control_get_y(target, min);
    lv_obj_set_pos(label_sv, bar_x - label_width - HMI_MARGIN, sv_y);
    target->label_sv = label_sv;

}

static void hmi_control_control_mode_cb(lv_obj_t *button, lv_event_t e)
{
    if (e == LV_EVENT_VALUE_CHANGED) {
        if (hmi_control_mode_callback != NULL) {
            uint16_t index = lv_btnmatrix_get_active_btn(button);

            ESP_LOGD(TAG, "hmi_control_control_mode_cb, index:%d", index);

            hmi_control_mode_callback(index);
        }
    }
}

static const char *hmi_control_mode_map[] = { "Off", "\n", "Manual", "\n", "Auto", "" };

static lv_obj_t* hmi_control_create_mode(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{

    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_clean_style_list(cont, LV_CONT_PART_MAIN);
    lv_obj_set_pos(cont, x, y);
    lv_obj_set_size(cont, w, h);

    lv_obj_t *label = lv_label_create(cont, NULL);
    lv_label_set_text(label, "Control");
    lv_obj_align(label, cont, LV_ALIGN_IN_TOP_MID, 0, HMI_MARGIN);

    lv_obj_t *matrix = lv_btnmatrix_create(cont, NULL);
    lv_obj_clean_style_list(matrix, LV_BTNMATRIX_PART_BG);

    lv_btnmatrix_set_map(matrix, hmi_control_mode_map);
    lv_btnmatrix_set_one_check(matrix, true);
    lv_btnmatrix_set_btn_ctrl(matrix, HMI_CONTROL_MODE_OFF, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(matrix, HMI_CONTROL_MODE_MANUAL, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(matrix, HMI_CONTROL_MODE_AUTO, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(matrix, HMI_CONTROL_MODE_OFF, LV_BTNMATRIX_CTRL_CHECK_STATE);

    // use remaining size
    lv_area_t label_coords;
    lv_obj_get_coords(label, &label_coords);
    lv_area_t cont_coords;
    lv_obj_get_coords(cont, &cont_coords);
    lv_obj_set_size(matrix, cont_coords.x2 - cont_coords.x1 - 10, cont_coords.y2 - label_coords.y2 - 10);
    lv_obj_align(matrix, label, LV_ALIGN_OUT_BOTTOM_MID, 0, HMI_MARGIN);

    lv_obj_set_event_cb(matrix, hmi_control_control_mode_cb);
    return matrix;
}

static void hmi_control_manual_cb(lv_obj_t *button, lv_event_t e)
{
    if (e == LV_EVENT_CLICKED) {

        uint16_t index = lv_btnmatrix_get_active_btn(button);
        bool checked = lv_btnmatrix_get_btn_ctrl(button, index, LV_BTNMATRIX_CTRL_CHECK_STATE);

        ESP_LOGD(TAG, "hmi_control_manual_cb, index:%d, checked:%d", index, checked);

        if (index == 0 && hmi_control_light_sv_callback != NULL) {
            hmi_control_light_sv_callback(checked);
        } else if (index == 1 && hmi_control_heater_sv_callback != NULL) {
            hmi_control_heater_sv_callback(checked);
        } else if (index == 2 && hmi_control_exhaust_sv_callback != NULL) {
            hmi_control_exhaust_sv_callback(checked);
        } else if (index == 3 && hmi_control_recirc_sv_callback != NULL) {
            hmi_control_recirc_sv_callback(checked);
        }
    }
}
static const char *hmi_control_manual_map[] = { "Light", "\n", "Heater", "\n", "Exhaust", "\n", "Recirc.", "" };

static lv_obj_t* hmi_control_create_manual(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{

    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_pos(cont, x, y);
    lv_obj_set_size(cont, w, h);
    lv_obj_clean_style_list(cont, LV_CONT_PART_MAIN);

    lv_obj_t *label = lv_label_create(cont, NULL);
    lv_label_set_text(label, "Manual");
    lv_obj_align(label, cont, LV_ALIGN_IN_TOP_MID, 0, HMI_MARGIN);

    lv_obj_t *matrix = lv_btnmatrix_create(cont, NULL);
    lv_obj_clean_style_list(matrix, LV_BTNMATRIX_PART_BG);

    lv_btnmatrix_set_map(matrix, hmi_control_manual_map);
    lv_btnmatrix_set_one_check(matrix, false);
    lv_btnmatrix_set_btn_ctrl(matrix, 0, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(matrix, 1, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(matrix, 2, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(matrix, 3, LV_BTNMATRIX_CTRL_CHECKABLE);

    // use remaining size
    lv_area_t label_coords;
    lv_obj_get_coords(label, &label_coords);
    lv_area_t cont_coords;
    lv_obj_get_coords(cont, &cont_coords);
    lv_obj_set_size(matrix, cont_coords.x2 - cont_coords.x1 - 10, cont_coords.y2 - label_coords.y2 - 10);
    lv_obj_align(matrix, label, LV_ALIGN_OUT_BOTTOM_MID, 0, HMI_MARGIN);

    lv_obj_set_event_cb(matrix, hmi_control_manual_cb);
    return matrix;
}

lv_obj_t* hmi_control_create_tab(lv_obj_t *parent)
{

    lv_obj_t *tab = lv_tabview_add_tab(parent, "Control");

    hmi_control_create_control(&hmi_control_temperature, tab, HMI_MARGIN,
    HMI_MARGIN,
    HMI_CONTROL_W, HMI_CONTROL_H, "Temperature [°C]", 0.0, 50.0, 1);
    hmi_control_set_pv(&hmi_control_temperature, 0.0);
    hmi_control_set_sv(&hmi_control_temperature, 0.0);
    hmi_control_set_hi(&hmi_control_temperature, false);
    hmi_control_set_lo(&hmi_control_temperature, false);

    hmi_control_create_control(&hmi_control_humidity, tab,
    HMI_MARGIN + HMI_CONTROL_W, HMI_MARGIN,
    HMI_CONTROL_W, HMI_CONTROL_H, "Humidity [%RH]", 0.0, 100.0, 1);
    hmi_control_set_pv(&hmi_control_humidity, 0.0);
    hmi_control_set_sv(&hmi_control_humidity, 0.0);
    hmi_control_set_hi(&hmi_control_humidity, false);
    hmi_control_set_lo(&hmi_control_humidity, false);

    hmi_control_create_control(&hmi_control_co2, tab, (HMI_MARGIN + HMI_CONTROL_W) * 2,
    HMI_MARGIN, HMI_CONTROL_W, HMI_CONTROL_H, "CO2 conc. [ppm]", 0.0, 2000.0, 0);
    hmi_control_set_pv(&hmi_control_co2, 0.0);
    hmi_control_set_sv(&hmi_control_co2, 0.0);
    hmi_control_set_hi(&hmi_control_co2, false);
    hmi_control_set_lo(&hmi_control_co2, false);

    hmi_control_mode_btnmatrix = hmi_control_create_mode(tab, (HMI_MARGIN + HMI_CONTROL_W) * 3 + 10, HMI_MARGIN, 110, 120);

    hmi_control_manual_btnmatrix = hmi_control_create_manual(tab, (HMI_MARGIN + HMI_CONTROL_W) * 3 + 10, 125, 110, 150);

    return tab;
}

void hmi_control_set_control_mode(hmi_control_mode_t mode)
{
    if (hmi_semaphore_take("hmi_control_set_control_mode")) {

        // select matching control mode button
        lv_btnmatrix_set_btn_ctrl(hmi_control_mode_btnmatrix, mode, LV_BTNMATRIX_CTRL_CLICK_TRIG);

        // disable manual control buttons
        if (mode == HMI_CONTROL_MODE_MANUAL) {
            lv_btnmatrix_clear_btn_ctrl_all(hmi_control_manual_btnmatrix, LV_BTNMATRIX_CTRL_DISABLED);
        } else {
            lv_btnmatrix_set_btn_ctrl_all(hmi_control_manual_btnmatrix, LV_BTNMATRIX_CTRL_DISABLED);
        }

        hmi_semaphore_give();
    }
}

void hmi_control_set_control_mode_callback(hmi_control_mode_callback_t callback)
{
    hmi_control_mode_callback = callback;
}

void hmi_control_set_temp_pv(double pv)
{
    // model in K, display in C
    hmi_control_set_pv(&hmi_control_temperature, pv - 273.15);
}

void hmi_control_set_temp_sv(double sv)
{
    hmi_control_set_sv(&hmi_control_temperature, sv);
}

void hmi_control_set_temp_hi(bool hi)
{
    hmi_control_set_hi(&hmi_control_temperature, hi);
}

void hmi_control_set_temp_lo(bool lo)
{
    hmi_control_set_lo(&hmi_control_temperature, lo);
}

void hmi_control_set_hum_pv(double pv)
{
    hmi_control_set_pv(&hmi_control_humidity, pv);
}

void hmi_control_set_hum_sv(double sv)
{
    hmi_control_set_sv(&hmi_control_humidity, sv);
}

void hmi_control_set_hum_hi(bool hi)
{
    hmi_control_set_hi(&hmi_control_humidity, hi);
}

void hmi_control_set_hum_lo(bool lo)
{
    hmi_control_set_lo(&hmi_control_humidity, lo);
}

void hmi_control_set_co2_pv(double pv)
{
    hmi_control_set_pv(&hmi_control_co2, pv);
}

void hmi_control_set_co2_sv(double sv)
{
    hmi_control_set_sv(&hmi_control_co2, sv);
}

void hmi_control_set_co2_hi(bool hi)
{
    hmi_control_set_hi(&hmi_control_co2, hi);
}

void hmi_control_set_co2_lo(bool lo)
{
    hmi_control_set_lo(&hmi_control_co2, lo);
}

static void hmi_control_set_manual_btn(uint16_t btn_id, bool active)
{
    if (hmi_semaphore_take("hmi_control_set_manual_btn")) {
        if (active) {
            lv_btnmatrix_set_btn_ctrl(hmi_control_manual_btnmatrix, btn_id, LV_BTNMATRIX_CTRL_CHECK_STATE);
        } else {
            lv_btnmatrix_clear_btn_ctrl(hmi_control_manual_btnmatrix, btn_id, LV_BTNMATRIX_CTRL_CHECK_STATE);
        }
        hmi_semaphore_give();
    }
}

void hmi_control_set_light_sv(bool active)
{
    hmi_control_set_manual_btn(0, active);
}

void hmi_control_set_light_sv_callback(hmi_bool_callback_t callback)
{
    hmi_control_light_sv_callback = callback;
}

void hmi_control_set_heater_sv(bool active)
{
    hmi_control_set_manual_btn(1, active);
}

void hmi_control_set_heater_sv_callback(hmi_bool_callback_t callback)
{
    hmi_control_heater_sv_callback = callback;
}

void hmi_control_set_exhaust_sv(bool active)
{
    hmi_control_set_manual_btn(2, active);
}

void hmi_control_set_exhaust_sv_callback(hmi_bool_callback_t callback)
{
    hmi_control_exhaust_sv_callback = callback;
}

void hmi_control_set_recirc_sv(bool active)
{
    hmi_control_set_manual_btn(3, active);
}

void hmi_control_set_recirc_sv_callback(hmi_bool_callback_t callback)
{
    hmi_control_recirc_sv_callback = callback;
}

