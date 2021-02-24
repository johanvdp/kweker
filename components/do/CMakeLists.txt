set(req driver esp32 freertos pubsub)

idf_component_register(
    SRCS "DO.cpp"
    INCLUDE_DIRS .
    REQUIRES ${req}
)