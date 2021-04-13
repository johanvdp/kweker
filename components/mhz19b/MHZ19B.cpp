// The author disclaims copyright to this source code.

#include "MHZ19B.h"

#include "hal/gpio_types.h"
#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "pubsub.h"


static const char *TAG = "MHZ19B";

MHZ19B::MHZ19B()
{
}

MHZ19B::~MHZ19B()
{
}

/**
 * Link C static world to C++ instance
 */
void MHZ19B::task(void *pvParameter)
{
    if (pvParameter == 0) {
        ESP_LOGE(TAG, "task, invalid pvParameter");
    } else {
        // should be an instance
        MHZ19B *pInstance = (MHZ19B*) pvParameter;
        pInstance->run();
    }
}

void MHZ19B::setup(gpio_num_t rx_pin, gpio_num_t tx_pin, const char *co2_topic)
{
    ESP_LOGD(TAG, "setup, rx_pin:%d, tx_pin:%d, topic_name:%s, this:%p", rx_pin, tx_pin, co2_topic, this);
    this->rx_pin = rx_pin;
    this->tx_pin = tx_pin;
    this->co2_topic = co2_topic;

    // start task
    esp_err_t ret = xTaskCreate(&task, TAG, 2048, this, tskIDLE_PRIORITY,
    NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "setup, xTaskCreate failed:%d (FATAL)", ret);
        return;
    }
}


/**
 * Run task for this instance.
 */
void MHZ19B::run()
{
    ESP_LOGD(TAG, "run, this:%p", this);

    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    };
}

