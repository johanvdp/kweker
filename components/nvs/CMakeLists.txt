set(req driver esp32 nvs_flash freertos pubsub)

idf_component_register(
    SRCS "NVS.cpp"
    INCLUDE_DIRS .
    REQUIRES ${req}
)