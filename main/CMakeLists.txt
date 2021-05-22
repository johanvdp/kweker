set(SOURCES main.cpp
			hmi.c
			hmi_about.c
			hmi_control.c
			hmi_settings.c
			hmi_datespinner.c
			hmi_timespinner.c
			hmi_numberspinner.c
			model.c
			bind.c
			bind_control.c
			bind_settings.c
			ctrl_circadian.c
			ctrl_day_night.c
			ctrl_auto.c
			ctrl_manual.c
			ctrl_off.c
			ctrl.c)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS .
                    REQUIRES driver esp32 freertos led am2301 lvgl lvgl_esp32_drivers pubsub led do ds3234 nvs mhz19b mcp23s17)

target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLV_LVGL_H_INCLUDE_SIMPLE")
