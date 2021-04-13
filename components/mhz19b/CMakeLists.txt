set(req driver esp32 freertos pubsub)

idf_component_register(
    SRCS "MHZ19B.cpp"
    INCLUDE_DIRS .
    REQUIRES ${req}
)