// The author disclaims copyright to this source code.

#include <time.h>

#include "esp_log.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "lvgl.h"
#include "lvgl_helpers.h"

#include "hmi.h"
#include "hmi_control.h"
#include "hmi_settings.h"
#include "hmi_about.h"

#define HMI_WIDTH 480
#define HMI_HEIGHT 320
#define HMI_LED_WIDTH 20
#define HMI_LED_HEIGHT 20
#define HMI_TOOLBAR_HEIGHT 32
#define HMI_BUTTON_WIDTH 65
#define HMI_BUTTON_HEIGHT 26
#define HMI_MARGIN 5

#define HMI_SEMAPHORE_TICKS (1000 / portTICK_PERIOD_MS)

static const char *TAG = "hmi";

/** current time [24h] */
static lv_obj_t *hmi_label_current_time;
/** current circadian [day, night] */
static lv_obj_t *hmi_label_circadian;
/** current control mode [off, manual, automatic] */
static lv_obj_t *hmi_label_control_mode;
/** current light switch state [off, on] */
static lv_obj_t *hmi_led_light_switch;
/** current heater switch state [off, on] */
static lv_obj_t *hmi_led_heater_switch;
/** current exhaust fan switch state [off, on] */
static lv_obj_t *hmi_led_exhaust_switch;
/** current recirculation fan switch state [off, on] */
static lv_obj_t *hmi_led_recirculation_switch;

/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
static SemaphoreHandle_t hmi_semaphore = 0;

static lv_color_t buf1[DISP_BUF_SIZE];
static lv_color_t buf2[DISP_BUF_SIZE];
static lv_disp_buf_t disp_buf;

/** tab page holder */
static lv_obj_t *hmi_tabview;
/** navigation buttons */
static lv_obj_t *hmi_btnmatrix_tabview;

static lv_obj_t* hmi_create_led(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
        const char *text)
{

    lv_obj_t *led = lv_led_create(parent, NULL);
    lv_obj_set_pos(led, x, y);
    lv_obj_set_size(led, HMI_LED_WIDTH, HMI_LED_HEIGHT);
    lv_obj_set_style_local_value_str(led, LV_LED_PART_MAIN, LV_STATE_DEFAULT,
            text);
    lv_led_off(led);

    return led;
}

static lv_obj_t* hmi_create_label(lv_obj_t *parent, uint16_t x, uint16_t y,
        const char *text)
{

    lv_obj_t *label = lv_label_create(parent, NULL);
    lv_obj_set_pos(label, x, y);
    lv_label_set_text(label, text);

    return label;
}

static void hmi_navigation_event_cb(lv_obj_t *button, lv_event_t e)
{

    if (e == LV_EVENT_CLICKED) {
        /** navigation button clicked */
        uint16_t id = lv_btnmatrix_get_active_btn(hmi_btnmatrix_tabview);
        if (id != LV_BTNMATRIX_BTN_NONE && id <= 2) {
            /** select corresponding tab */
            lv_tabview_set_tab_act(hmi_tabview, id, LV_ANIM_OFF);
        }
    }
}

static const char *hmi_navigation_map[] = { "Control", "Settings", "About", "" };

static lv_obj_t* hmi_create_navigation_btnmatrix(lv_obj_t *parent, lv_coord_t x,
        lv_coord_t y, lv_coord_t w, lv_coord_t h)
{

    lv_obj_t *matrix = lv_btnmatrix_create(parent, NULL);
    // intentionally chop of lower edge of buttons
    lv_obj_set_pos(matrix, x, y + HMI_MARGIN);
    lv_obj_set_size(matrix, w, h);
    lv_obj_clean_style_list(matrix, LV_BTNMATRIX_PART_BG);

    lv_btnmatrix_set_map(matrix, hmi_navigation_map);
    lv_btnmatrix_set_one_check(matrix, true);
    lv_btnmatrix_set_btn_ctrl(matrix, 0, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(matrix, 1, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(matrix, 2, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(matrix, 0, LV_BTNMATRIX_CTRL_CHECK_STATE);

    lv_obj_set_event_cb(matrix, hmi_navigation_event_cb);

    return matrix;
}

static lv_obj_t* hmi_create_toolbar(lv_obj_t *parent)
{

    lv_obj_t *toolbar = lv_obj_create(parent, NULL);
    lv_obj_clean_style_list(toolbar, LV_OBJ_PART_MAIN);
    uint16_t parent_width = lv_obj_get_width(parent);
    lv_obj_set_size(toolbar, parent_width, HMI_TOOLBAR_HEIGHT);

    hmi_label_current_time = hmi_create_label(toolbar, 8, 8, "--:--");
    hmi_label_circadian = hmi_create_label(toolbar, 48, 2, "DAY");
    hmi_label_control_mode = hmi_create_label(toolbar, 48, 16, "OFF");

    hmi_btnmatrix_tabview = hmi_create_navigation_btnmatrix(toolbar, 100, 0,
            250, HMI_TOOLBAR_HEIGHT);

    hmi_led_light_switch = hmi_create_led(toolbar,
            parent_width - (HMI_LED_WIDTH + HMI_MARGIN) * 4, HMI_MARGIN, "L");

    hmi_led_heater_switch = hmi_create_led(toolbar,
            parent_width - (HMI_LED_WIDTH + HMI_MARGIN) * 3, HMI_MARGIN, "H");

    hmi_led_exhaust_switch = hmi_create_led(toolbar,
            parent_width - (HMI_LED_WIDTH + HMI_MARGIN) * 2, HMI_MARGIN, "E");

    hmi_led_recirculation_switch = hmi_create_led(toolbar,
            parent_width - (HMI_LED_WIDTH + HMI_MARGIN), HMI_MARGIN, "R");

    return toolbar;
}

static void hmi_tabview_event_cb(lv_obj_t *button, lv_event_t e)
{

    if (e == LV_EVENT_VALUE_CHANGED) {
        /** tabview slide to other tab */
        uint16_t id = lv_tabview_get_tab_act(hmi_tabview);
        if (id != LV_BTNMATRIX_BTN_NONE && id <= 2) {
            /** synchronize selected navigation button */
            lv_btnmatrix_set_btn_ctrl(hmi_btnmatrix_tabview, id,
                    LV_BTNMATRIX_CTRL_CHECK_STATE);
        }
    }
}

static lv_obj_t* hmi_create_tabview(lv_obj_t *parent)
{

    lv_obj_t *tabview = lv_tabview_create(parent, NULL);
    lv_obj_set_pos(tabview, 0, HMI_TOOLBAR_HEIGHT);
    uint16_t parent_height = lv_obj_get_height(parent);
    uint16_t parent_width = lv_obj_get_width(parent);
    lv_obj_set_size(tabview, parent_width, parent_height - HMI_TOOLBAR_HEIGHT);
    lv_tabview_set_btns_pos(tabview, LV_TABVIEW_TAB_POS_NONE);
    lv_obj_set_event_cb(tabview, hmi_tabview_event_cb);
    return tabview;
}

static void hmi_task(void *pvParameter)
{
    while (1) {
        vTaskDelay(1);

        if (hmi_semaphore_take("hmi_task")) {
            lv_task_handler();
            hmi_semaphore_give();
        }
    }

    /* A task should NEVER return */
    vTaskDelete(NULL);
}

void hmi_initialize()
{

    hmi_semaphore = xSemaphoreCreateMutex();
    lv_init();

    /* the display driver */
    lvgl_driver_init();
    lv_disp_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /* the touch controller as input device */
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);

    /** the screen layout */
    lv_obj_t *screen = lv_obj_create(NULL, NULL);
    lv_scr_load(screen);

    hmi_create_toolbar(screen);
    hmi_tabview = hmi_create_tabview(screen);

    hmi_control_create_tab(hmi_tabview);
    hmi_settings_create_tab(hmi_tabview);
    hmi_about_create_tab(hmi_tabview);

    /* If you want to use a task to create the graphic, you NEED to create a Pinned task
     * Otherwise there can be problem such as memory corruption and so on.
     * NOTE: When not using Wi-Fi nor Bluetooth you can pin the guiTask to core 0 */
    xTaskCreatePinnedToCore(&hmi_task, TAG, 4096 * 2, NULL, 0, NULL, 1);
}

void hmi_set_current_time(time_t timestamp)
{
    if (hmi_semaphore_take("hmi_set_current_time")) {

        ESP_LOGD(TAG, "hmi_set_current_time time:%ld", timestamp);
        struct tm brokentime;
        gmtime_r(&timestamp, &brokentime);
        // HH:MM\0
        char text[] = { 0, 0, 0, 0, 0, 0 };
        snprintf(text, sizeof text, "%02d:%02d", brokentime.tm_hour,
                brokentime.tm_min);
        ESP_LOGI(TAG, "hmi_set_current_time time:%s", text);
        lv_label_set_text(hmi_label_current_time, text);

        hmi_semaphore_give();
    }
}

bool hmi_semaphore_take(const char *function_name)
{
    bool success;
    if (xSemaphoreTake(hmi_semaphore, HMI_SEMAPHORE_TICKS)) {
        success = true;
    } else {
        success = false;
        ESP_LOGE(TAG, "%s xSemaphoreTake failed", function_name);
    }
    return success;
}

void hmi_semaphore_give()
{
    xSemaphoreGive(hmi_semaphore);
}

void hmi_set_control_mode(hmi_control_mode_t mode)
{
    if (hmi_semaphore_take("hmi_set_control_mode")) {
        if (mode == HMI_CONTROL_MODE_OFF) {
            lv_label_set_text(hmi_label_control_mode, "OFF");
        } else if (mode == HMI_CONTROL_MODE_MANUAL) {
            lv_label_set_text(hmi_label_control_mode, "MANUAL");
        } else if (mode == HMI_CONTROL_MODE_AUTO) {
            lv_label_set_text(hmi_label_control_mode, "AUTO");
        }
        hmi_semaphore_give();
    }
}

void hmi_set_circadian(bool day)
{
    if (hmi_semaphore_take("hmi_set_circadian")) {

        ESP_LOGD(TAG, "hmi_set_circadian day:%s", day ? "true" : "false");
        lv_label_set_text(hmi_label_circadian, day ? "DAY" : "NIGHT");

        hmi_semaphore_give();
    }
}

static void hmi_set_led(lv_obj_t* led, bool active)
{
    if (hmi_semaphore_take("hmi_set_led")) {
        if (active) {
            lv_led_on(led);
        } else {
            lv_led_off(led);
        }

        hmi_semaphore_give();
    }
}

void hmi_set_light(bool active)
{
    hmi_set_led(hmi_led_light_switch, active);
}

void hmi_set_exhaust(bool active)
{
    hmi_set_led(hmi_led_exhaust_switch, active);
}

void hmi_set_heater(bool active)
{
    hmi_set_led(hmi_led_heater_switch, active);
}

void hmi_set_recirc(bool active)
{
    hmi_set_led(hmi_led_recirculation_switch, active);
}

