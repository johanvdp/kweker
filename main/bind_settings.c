// The author disclaims copyright to this source code.
#include "bind_settings.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "pubsub.h"
#include "model.h"
#include "hmi_settings.h"

static const char *TAG = "bind_settings";

/** time setting */
static QueueHandle_t bind_time;

static void bind_settings_task(void *pvParameter)
{
    pubsub_message_t message;
    while (true) {
        if (xQueueReceive(bind_time, &message, 0)) {
            hmi_settings_set_clock(message.int_val);
        }
        vTaskDelay(1);
    };
}

static void bind_settings_subscribe()
{
    bind_time = xQueueCreate(2, sizeof(pubsub_message_t));
    pubsub_add_subscription(bind_time, MODEL_TIME, true);
}

void bind_settings_initialize()
{
    bind_settings_subscribe();

    BaseType_t ret = xTaskCreate(&bind_settings_task, TAG, 2048, NULL,
            (tskIDLE_PRIORITY + 1), NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "failed to create task (FATAL)");
    }
}
