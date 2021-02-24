set(req driver esp32 freertos pubsub)

idf_component_register(
    SRCS "AM2301.cpp"
    INCLUDE_DIRS .
    REQUIRES ${req}
)