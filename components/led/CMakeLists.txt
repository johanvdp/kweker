set(req driver esp32 freertos)

idf_component_register(
    SRCS "LED.cpp"
    INCLUDE_DIRS .
    REQUIRES ${req}
)