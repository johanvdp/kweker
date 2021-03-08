// The author disclaims copyright to this source code.
#include "DS3234.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_err.h"
#include "time.h"
#include "pubsub.h"


/** Clock update interval (ms) */
#define DS3432_LOOK_INTERVAL_MS 60000

static const char *TAG = "DS3234";

DS3234::DS3234()
{
}

DS3234::~DS3234()
{
}

/**
 * Link C static world to C++ instance
 */
void DS3234::task(TimerHandle_t xTimer)
{
    if (xTimer == 0) {
        ESP_LOGE(TAG, "task, invalid timer");
    } else {
        // should be an instance
        DS3234 *pInstance = (DS3234*)pvTimerGetTimerID(xTimer);
        pInstance->run();
    }
}

void DS3234::setup(pubsub_topic_t topic)
{
    ESP_LOGD(TAG, "setup, topic: %p", topic);

    this->timestamp_topic = topic;

    TimerHandle_t timer = xTimerCreate("ds3234", (DS3432_LOOK_INTERVAL_MS / portTICK_RATE_MS), pdTRUE,
            (void *)this, &task);
    if (timer == NULL) {
        ESP_LOGE(TAG, "setup xTimerCreate failed (FATAL)");
        return;
    }

    xTimerStart(timer, 0);
}

/**
 * Run task for this instance.
 */
void DS3234::run()
{
    time_t t;
    time(&t);
    int64_t longtime = (int64_t)t;
    pubsub_publish_int(timestamp_topic, longtime);
}

