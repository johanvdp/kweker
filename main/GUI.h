// The author disclaims copyright to this source code.
#ifndef _GUI_H
#define _GUI_H

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include "lvgl.h"
#include "lvgl_helpers.h"
#include "Callback.h"

#define LV_TICK_PERIOD_MS 1

class GUI {

public:
	GUI();
	virtual ~GUI();
	void setup();

private:
	/* Creates a semaphore to handle concurrent call to lvgl stuff
	 * If you wish to call *any* lvgl function from other threads/tasks
	 * you should lock on the very same semaphore! */
	SemaphoreHandle_t xGuiSemaphore = 0;

	lv_group_t*  g;
	lv_obj_t * tv;
	lv_obj_t * t1;
	lv_obj_t * t2;
	lv_obj_t * t3;

	struct {
	    lv_obj_t * btn;
	    lv_obj_t * cb;
	    lv_obj_t * slider;
	    lv_obj_t * sw;
	    lv_obj_t * spinbox;
	    lv_obj_t * dropdown;
	    lv_obj_t * roller;
	    lv_obj_t * list;
	} selector_objs;

	struct {
	    lv_obj_t * ta1;
	    lv_obj_t * ta2;
	    lv_obj_t * kb;
	} textinput_objs;

	lv_color_t buf1[DISP_BUF_SIZE];
	lv_color_t buf2[DISP_BUF_SIZE];
	lv_disp_buf_t disp_buf;

	void run();
	void layout();
	void selectors_create(lv_obj_t * parent);
	void text_input_create(lv_obj_t * parent);
	void msgbox_create(void);

	// callbacks

	void focus_cb(lv_group_t * g);
	void msgbox_event_cb(lv_obj_t * msgbox, lv_event_t e);
	void tv_event_cb(lv_obj_t * ta, lv_event_t e);
	void ta_event_cb(lv_obj_t * ta, lv_event_t e);
	void kb_event_cb(lv_obj_t * kb, lv_event_t e);
	void dd_enc(lv_obj_t * obj, lv_event_t e);

	static void task(void *pvParameter);
	static void lv_tick_task(void *pvParameter);
};

#endif
