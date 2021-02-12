set(req driver esp32 freertos)

idf_component_register(
    SRCS "pubsub.c" "pubsub_test.c"
    INCLUDE_DIRS .
    REQUIRES ${req}
)