// The author disclaims copyright to this source code.
#include "GUI.h"

#define TAG "GUI"

GUI::GUI() {

}

GUI::~GUI() {

}

void GUI::setup() {
	ESP_LOGI(TAG, "setup");

	xGuiSemaphore = xSemaphoreCreateMutex();

	lv_init();

	/* Register the display driver */
	lvgl_driver_init();
	lv_disp_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.flush_cb = disp_driver_flush;
	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);

	/* Register the touch controller as input device */
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.read_cb = touch_driver_read;
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	lv_indev_drv_register(&indev_drv);

	/* Create and start a periodic timer interrupt to call lv_tick_inc */
	const esp_timer_create_args_t periodic_timer_args = { //
			.callback = &lv_tick_task, //
					.arg = NULL, //
					.dispatch_method = ESP_TIMER_TASK, //
					.name = "periodic_gui", };
	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	ESP_ERROR_CHECK(
			esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

	layout();

	/* If you want to use a task to create the graphic, you NEED to create a Pinned task
	 * Otherwise there can be problem such as memory corruption and so on.
	 * NOTE: When not using Wi-Fi nor Bluetooth you can pin the guiTask to core 0 */
	xTaskCreatePinnedToCore(&task, "gui", 4096 * 2, this, 0, NULL, 1);
}

void GUI::layout() {
	g = lv_group_create();

	lv_group_focus_cb_t c_focus_cb = GETCB(lv_group_focus_cb_t, GUI)(
			std::bind(&GUI::focus_cb, this, std::placeholders::_1));
	lv_group_set_focus_cb(g, c_focus_cb);

	tv = lv_tabview_create(lv_scr_act(), NULL);

	lv_event_cb_t c_tv_event_cb = GETCB(lv_event_cb_t, GUI)(
			std::bind(&GUI::tv_event_cb, this, std::placeholders::_1,
					std::placeholders::_2));
	lv_obj_set_event_cb(tv, c_tv_event_cb);

	t1 = lv_tabview_add_tab(tv, "Selectors");
	t2 = lv_tabview_add_tab(tv, "Text input");

	lv_group_add_obj(g, tv);

	selectors_create(t1);
	text_input_create(t2);

	msgbox_create();
}

void GUI::dd_enc(lv_obj_t *obj, lv_event_t e) {
	ESP_LOGI(TAG, "dd_enc");

	if (e == LV_EVENT_VALUE_CHANGED) {
		/*printf("chg\n");*/
	}
}

void GUI::selectors_create(lv_obj_t *parent) {
	ESP_LOGI(TAG, "selectors_create");

	lv_page_set_scrl_layout(parent, LV_LAYOUT_COLUMN_MID);

	selector_objs.btn = lv_btn_create(parent, NULL);

	lv_obj_t *label = lv_label_create(selector_objs.btn, NULL);
	lv_label_set_text(label, "Button");

	selector_objs.cb = lv_checkbox_create(parent, NULL);

	selector_objs.slider = lv_slider_create(parent, NULL);
	lv_slider_set_range(selector_objs.slider, 0, 10);

	selector_objs.sw = lv_switch_create(parent, NULL);

	selector_objs.spinbox = lv_spinbox_create(parent, NULL);

	selector_objs.dropdown = lv_dropdown_create(parent, NULL);

	lv_event_cb_t c_dd_enc = GETCB(lv_event_cb_t, GUI)(
			std::bind(&GUI::dd_enc, this, std::placeholders::_1,
					std::placeholders::_2));
	lv_obj_set_event_cb(selector_objs.dropdown, c_dd_enc);

	selector_objs.roller = lv_roller_create(parent, NULL);

	selector_objs.list = lv_list_create(parent, NULL);
	if (lv_obj_get_height(selector_objs.list)
			> lv_page_get_height_fit(parent)) {
		lv_obj_set_height(selector_objs.list, lv_page_get_height_fit(parent));
	}
	lv_list_add_btn(selector_objs.list, LV_SYMBOL_OK, "Apply");
	lv_list_add_btn(selector_objs.list, LV_SYMBOL_CLOSE, "Close");
	lv_list_add_btn(selector_objs.list, LV_SYMBOL_EYE_OPEN, "Show");
	lv_list_add_btn(selector_objs.list, LV_SYMBOL_EYE_CLOSE, "Hide");
	lv_list_add_btn(selector_objs.list, LV_SYMBOL_TRASH, "Delete");
	lv_list_add_btn(selector_objs.list, LV_SYMBOL_COPY, "Copy");
	lv_list_add_btn(selector_objs.list, LV_SYMBOL_PASTE, "Paste");
}

void GUI::text_input_create(lv_obj_t *parent) {
	ESP_LOGI(TAG, "text_input_create");

	textinput_objs.ta1 = lv_textarea_create(parent, NULL);

	lv_event_cb_t c_ta_event_cb = GETCB(lv_event_cb_t, GUI)(
			std::bind(&GUI::ta_event_cb, this, std::placeholders::_1,
					std::placeholders::_2));
	lv_obj_set_event_cb(textinput_objs.ta1, c_ta_event_cb);

	lv_obj_align(textinput_objs.ta1, NULL, LV_ALIGN_IN_TOP_MID, 0, LV_DPI / 20);
	lv_textarea_set_one_line(textinput_objs.ta1, true);
	lv_textarea_set_cursor_hidden(textinput_objs.ta1, true);
	lv_textarea_set_placeholder_text(textinput_objs.ta1, "Type something");
	lv_textarea_set_text(textinput_objs.ta1, "");

	textinput_objs.ta2 = lv_textarea_create(parent, textinput_objs.ta1);
	lv_obj_align(textinput_objs.ta2, textinput_objs.ta1,
			LV_ALIGN_OUT_BOTTOM_MID, 0, LV_DPI / 20);

	textinput_objs.kb = NULL;
}

void GUI::msgbox_create(void) {
	ESP_LOGI(TAG, "msgbox_create");

	lv_obj_t *mbox = lv_msgbox_create(lv_layer_top(), NULL);
	lv_msgbox_set_text(mbox, "Welcome to the keyboard and encoder demo");

	lv_event_cb_t c_msgbox_event_cb = GETCB(lv_event_cb_t, GUI)(
			std::bind(&GUI::msgbox_event_cb, this, std::placeholders::_1,
					std::placeholders::_2));
	lv_obj_set_event_cb(mbox, c_msgbox_event_cb);

	lv_group_add_obj(g, mbox);
	lv_group_focus_obj(mbox);
	lv_group_focus_freeze(g, true);

	static const char *btns[] = { "Ok", "Cancel", "" };
	lv_msgbox_add_btns(mbox, btns);
	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);

	lv_obj_set_style_local_bg_opa(lv_layer_top(), LV_OBJ_PART_MAIN,
			LV_STATE_DEFAULT, LV_OPA_70);
	lv_obj_set_style_local_bg_color(lv_layer_top(), LV_OBJ_PART_MAIN,
			LV_STATE_DEFAULT, LV_COLOR_GRAY);
	lv_obj_set_click(lv_layer_top(), true);
}

void GUI::msgbox_event_cb(lv_obj_t *msgbox, lv_event_t e) {
	ESP_LOGI(TAG, "msgbox_event_cb");

	if (e == LV_EVENT_CLICKED) {
		uint16_t b = lv_msgbox_get_active_btn(msgbox);
		if (b == 0 || b == 1) {
			lv_obj_del(msgbox);
			lv_obj_reset_style_list(lv_layer_top(), LV_OBJ_PART_MAIN);
			lv_obj_set_click(lv_layer_top(), false);
			lv_event_send(tv, LV_EVENT_REFRESH, NULL);
		}
	}
}

void GUI::focus_cb(lv_group_t *group) {
	ESP_LOGI(TAG, "focus_cb");

	lv_obj_t *obj = lv_group_get_focused(group);
	if (obj != tv) {
		uint16_t tab = lv_tabview_get_tab_act(tv);
		switch (tab) {
		case 0:
			lv_page_focus(t1, obj, LV_ANIM_ON);
			break;
		case 1:
			lv_page_focus(t2, obj, LV_ANIM_ON);
			break;
		case 2:
			lv_page_focus(t3, obj, LV_ANIM_ON);
			break;
		}
	}
}

void GUI::tv_event_cb(lv_obj_t *ta, lv_event_t e) {
	ESP_LOGI(TAG, "tv_event_cb");

	if (e == LV_EVENT_VALUE_CHANGED || e == LV_EVENT_REFRESH) {
		lv_group_remove_all_objs(g);

		uint16_t tab = lv_tabview_get_tab_act(tv);
		size_t size = 0;
		lv_obj_t **objs = NULL;
		if (tab == 0) {
			size = sizeof(selector_objs);
			objs = (lv_obj_t**) &selector_objs;
		} else if (tab == 1) {
			size = sizeof(textinput_objs);
			objs = (lv_obj_t**) &textinput_objs;
		}

		lv_group_add_obj(g, tv);

		uint32_t i;
		for (i = 0; i < size / sizeof(lv_obj_t*); i++) {
			if (objs[i] == NULL)
				continue;
			lv_group_add_obj(g, objs[i]);
		}

	}

}

void GUI::ta_event_cb(lv_obj_t *ta, lv_event_t e) {
	ESP_LOGI(TAG, "ta_event_cb");

	/*Create a virtual keyboard for the encoders*/
	lv_indev_t *indev = lv_indev_get_act();
	if (indev == NULL)
		return;
	lv_indev_type_t indev_type = lv_indev_get_type(indev);

	if (e == LV_EVENT_FOCUSED) {
		lv_textarea_set_cursor_hidden(ta, false);
		if (lv_group_get_editing(g)) {
			if (textinput_objs.kb == NULL) {
				textinput_objs.kb = lv_keyboard_create(lv_scr_act(), NULL);
				lv_group_add_obj(g, textinput_objs.kb);

				lv_event_cb_t c_kb_event_cb = GETCB(lv_event_cb_t, GUI)(
						std::bind(&GUI::kb_event_cb, this,
								std::placeholders::_1, std::placeholders::_2));
				lv_obj_set_event_cb(textinput_objs.kb, c_kb_event_cb);

				lv_obj_set_height(tv,
				LV_VER_RES - lv_obj_get_height(textinput_objs.kb));
			}

			lv_keyboard_set_textarea(textinput_objs.kb, ta);
			lv_group_focus_obj(textinput_objs.kb);
			lv_group_set_editing(g, true);
			lv_page_focus(t2, lv_textarea_get_label(ta), LV_ANIM_ON);
		}
	} else if (e == LV_EVENT_DEFOCUSED) {
		if (indev_type == LV_INDEV_TYPE_ENCODER) {
			if (textinput_objs.kb == NULL) {
				lv_textarea_set_cursor_hidden(ta, true);
			}
		} else {
			lv_textarea_set_cursor_hidden(ta, true);
		}
	}
}

void GUI::kb_event_cb(lv_obj_t *kb, lv_event_t e) {
	ESP_LOGI(TAG, "kb_event_cb");

	lv_keyboard_def_event_cb(kb, e);

	if (e == LV_EVENT_APPLY || e == LV_EVENT_CANCEL) {
		lv_group_focus_obj(lv_keyboard_get_textarea(kb));
		lv_obj_del(kb);
		textinput_objs.kb = NULL;
		lv_obj_set_height(tv, LV_VER_RES);
	}
}

void GUI::run() {
	while (1) {
		vTaskDelay(1);

		if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
			lv_task_handler();
			xSemaphoreGive(xGuiSemaphore);
		}
	}

	/* A task should NEVER return */
	vTaskDelete(NULL);
}

/**
 * Link C static world to C++ world
 */
void GUI::task(void *pvParameter) {
	ESP_LOGI(TAG, "task");
	if (pvParameter == 0) {
		ESP_LOGE(TAG, "task, invalid parameter (FATAL)");
	} else {
		GUI *pInstance = (GUI*) pvParameter;
		pInstance->run();
	}
}

void GUI::lv_tick_task(void *pvParameter) {
	lv_tick_inc(LV_TICK_PERIOD_MS);
}

