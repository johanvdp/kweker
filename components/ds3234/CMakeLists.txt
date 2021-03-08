set(req driver esp32 freertos pubsub)

idf_component_register(
    SRCS "DS3234.cpp"
    INCLUDE_DIRS .
    REQUIRES ${req}
)