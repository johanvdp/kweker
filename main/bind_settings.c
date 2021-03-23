// The author disclaims copyright to this source code.
#include "bind_settings.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "pubsub.h"
#include "model.h"
#include "hmi_settings.h"

static const char *TAG = "bind_control";

static void bind_settings_task(void *pvParameter)
{
    pubsub_message_t message;
    while (true) {
        vTaskDelay(1);
    };
}

void bind_settings_initialize()
{

    BaseType_t ret = xTaskCreate(&bind_settings_task, TAG, 2048, NULL,
            (tskIDLE_PRIORITY + 1), NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "failed to create task (FATAL)");
    }
}
