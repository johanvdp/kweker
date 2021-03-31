// The author disclaims copyright to this source code.

#include <stdbool.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pubsub.h"

#include "model.h"

#include "ctrl_circadian.h"
#include "ctrl_day_night.h"
#include "ctrl_auto.h"
#include "ctrl_manual.h"
#include "ctrl_off.h"

static const char *TAG = "ctrl";

static void ctrl_task(void *pvParameter)
{
    while (true) {
        ctrl_circadian_task();
        ctrl_day_night_task();
        ctrl_auto_task();
        ctrl_manual_task();
        ctrl_off_task();

        vTaskDelay(1);
    };
}

void ctrl_initialize()
{
    ctrl_circadian_initialize();
    ctrl_day_night_initialize();
    ctrl_auto_initialize();
    ctrl_manual_initialize();
    ctrl_off_initialize();

    BaseType_t ret = xTaskCreate(&ctrl_task, TAG, 2048, NULL,
            (tskIDLE_PRIORITY + 1), NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "failed to create task (FATAL)");
    }
}

