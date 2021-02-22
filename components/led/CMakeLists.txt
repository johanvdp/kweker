set(req driver esp32 freertos pubsub)

idf_component_register(
    SRCS "LED.cpp"
    INCLUDE_DIRS .
    REQUIRES ${req}
)