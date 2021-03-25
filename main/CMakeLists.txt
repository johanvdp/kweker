set(SOURCES main.cpp hmi.c hmi_about.c hmi_control.c hmi_settings.c hmi_timespinner.c model.c bind.c bind_control.c bind_settings.c)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS .
                    REQUIRES driver esp32 freertos led am2301 lvgl lvgl_esp32_drivers pubsub led do ds3234)

target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLV_LVGL_H_INCLUDE_SIMPLE")
