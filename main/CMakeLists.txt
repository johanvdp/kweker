set(SOURCES main.cpp hmi_about.c hmi_control.c hmi_settings.c hmi.c)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS .
                    REQUIRES led am2301 lvgl lvgl_esp32_drivers pubsub)

target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLV_LVGL_H_INCLUDE_SIMPLE")
