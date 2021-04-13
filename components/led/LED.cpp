// The author disclaims copyright to this source code.

#include "LED.h"

static const char *TAG = "LED";

LED::LED()
{
}

LED::~LED()
{
}

void LED::setup(gpio_num_t pin, bool on, const char *topic)
{
    ESP_LOGD(TAG, "setup, pin:%d, on:%d", pin, on);

    if (pin < GPIO_NUM_0 || pin >= GPIO_NUM_MAX) {
        ESP_LOGE(TAG, "setup requires GPIO pin number (FATAL)");
        return;
    }
    if (topic == 0) {
        ESP_LOGE(TAG, "setup requires topic (FATAL)");
        return;
    }

    // big queue not useful
    QueueHandle_t led_queue = xQueueCreate(10, sizeof(pubsub_message_t));
    if (led_queue == 0) {
        ESP_LOGE(TAG, "setup, failed to create queue (FATAL)");
        return;
    }
    pubsub_add_subscription(led_queue, topic, false);

    this->pin = pin;
    this->on = on;
    this->queue = led_queue;

    gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    BaseType_t ret = xTaskCreate(&task, TAG, 2048, this, (tskIDLE_PRIORITY + 2), NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "setup, failed to create task (FATAL)");
    }
}

void LED::run()
{
    pubsub_message_t message;
    while (true) {
        if (xQueueReceive(queue, &message, portMAX_DELAY)) {
            ESP_LOGD(TAG, "run, blink %lld", message.int_val);
            for (int i = 0; i < message.int_val; i++) {
                gpio_set_level(pin, on);
                vTaskDelay(20 / portTICK_PERIOD_MS);
                gpio_set_level(pin, !on);
                vTaskDelay(80 / portTICK_PERIOD_MS);
            }
        }
    };
}

/**
 * Link C static world to C++ world
 */
void LED::task(void *pvParameter)
{
    if (pvParameter == 0) {
        ESP_LOGE(TAG, "task, invalid parameter (FATAL)");
    } else {
        LED *pInstance = (LED*) pvParameter;
        pInstance->run();
    }
}
