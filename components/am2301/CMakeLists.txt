set(req driver esp32 freertos)

idf_component_register(
    SRCS "AM2301.cpp"
    INCLUDE_DIRS .
    REQUIRES ${req}
)