set(SOURCES main.cpp hmi.c hmi_about.c hmi_control.c hmi_settings.c model.c)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS .
                    REQUIRES led am2301 lvgl lvgl_esp32_drivers pubsub led do)

target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLV_LVGL_H_INCLUDE_SIMPLE")
