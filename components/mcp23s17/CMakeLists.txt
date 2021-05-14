set(req driver esp32 freertos pubsub)

idf_component_register(
    SRCS "MCP23S17.cpp"
    INCLUDE_DIRS .
    REQUIRES ${req}
)