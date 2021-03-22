// The author disclaims copyright to this source code.
#include "AM2301.h"

static const char *TAG = "AM2301";

AM2301::AM2301()
{
}

AM2301::~AM2301()
{
}

void AM2301::setup(gpio_num_t pin, pubsub_topic_t temperature_topic,
        pubsub_topic_t humidity_topic, pubsub_topic_t status_topic,
        pubsub_topic_t timestamp_topic)
{
    ESP_LOGD(TAG, "setup, pin:%d, t:%p, rh:%p, status:%p, time:%p", pin, temperature_topic, humidity_topic, status_topic, timestamp_topic);

    if (state != COMPONENT_UNINITIALIZED) {
        state = COMPONENT_FATAL;
        ESP_LOGE(TAG, "setup only once (FATAL)");
        return;
    }

    if (pin < GPIO_NUM_0 || pin >= GPIO_NUM_MAX) {
        state = COMPONENT_FATAL;
        ESP_LOGE(TAG, "setup requires GPIO pin number (FATAL)");
        return;
    }
    this->pin = pin;

    this->temperature_topic = temperature_topic;
    this->humidity_topic = humidity_topic;
    this->status_topic = status_topic;
    this->timestamp_topic = timestamp_topic;

    decoderQueue = xQueueCreate(NUMBER_OF_EDGES_IN_DATA_FRAME,
            sizeof(decoder_data_t));
    if (decoderQueue == 0) {
        state = COMPONENT_FATAL;
        ESP_LOGE(TAG, "setup xQueueCreate failed (FATAL)");
        return;
    }

    BaseType_t ret = xTaskCreatePinnedToCore(&task, TAG, 2048, this,
            (4 | portPRIVILEGE_BIT), NULL, 1);
    if (ret != pdPASS) {
        state = COMPONENT_FATAL;
        ESP_LOGE(TAG, "setup xTaskCreate failed:%d (FATAL)", ret);
        return;
    }

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        state = COMPONENT_FATAL;
        ESP_LOGE(TAG, "setup gpio_config failed:%d (FATAL)", ret);
        return;
    }

    // using gpio_isr_handler_add
    // assumes that GPIO ISR handling is installed
    //
    //    BaseType_t ret = gpio_install_isr_service(
    //    ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM);
    //    if (ret != ESP_OK) {
    //        ESP_LOGE(TAG, "gpio_install_isr_service failed:%d (FATAL)", ret);
    //        return;
    //    }
    ret = gpio_isr_handler_add(pin, isr_handler, this);
    if (ret != ESP_OK) {
        state = COMPONENT_FATAL;
        ESP_LOGE(TAG, "setup gpio_isr_handler_add failed:%d (FATAL)", ret);
        return;
    }

    state = COMPONENT_READY;
}

bool AM2301::measure()
{
    ESP_LOGD(TAG, "measure");

    if (state == COMPONENT_FATAL) {
        ESP_LOGE(TAG, "measure component state (FATAL)");
        return false;
    } else if (state == COMPONENT_UNINITIALIZED) {
        ESP_LOGE(TAG, "measure component not setup");
        return false;
    } else if (state != COMPONENT_READY) {
        ESP_LOGE(TAG, "measure component not ready");
        return false;
    }

    return queue_instruction_start();
}

/**
 * Put INSTRUCTION_START into the queue.
 *
 * @return True if adding was succesful. False if not (queue full).
 */
bool AM2301::queue_instruction_start()
{
    ESP_LOGD(TAG, "queue_instruction_start");

    decoder_data_t data;
    data.instruction = INSTRUCTION_START;
    BaseType_t ret = xQueueSend(decoderQueue, &data, (TickType_t ) 0);
    if (ret != pdPASS) {
        // queue full, might recover
        ESP_LOGE(TAG, "queue_instruction_start xQueueSend failed");
        return false;
    }

    return true;
}

/**
 * listen to one wire bus.
 * - pin as input with pull-up
 */
void AM2301::one_wire_listen()
{
    gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
    gpio_set_direction(pin, GPIO_MODE_INPUT);
}

/**
 * float one wire bus with pull-up.
 * - pin as open drain output
 * - release bus
 */
void AM2301::one_wire_float()
{
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 1);
}

/**
 * send one wire bus start pulse.
 * - pin as open drain output
 * - pull bus low to start measurement
 * - low Tbe = typ 1 ms (max 20 ms)
 * - release bus
 */
void AM2301::one_wire_start()
{
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 0);
    vTaskDelay(1);
    gpio_set_level(pin, 1);
}

void AM2301::handle_instruction_start()
{
    ESP_LOGD(TAG, "handle_instruction_start");

    state = HOST_SEND_START;
    data = 0;
    bit = 40;
    one_wire_start();
    one_wire_listen();
    state = WAIT_FOR_DEVICE_START;
}

void AM2301::handle_instruction_edge_detected()
{
    ESP_LOGV(TAG, "state:%d, level:%d", state, decoderData.level);

    if (state == WAIT_FOR_DEVICE_START) {
        if (decoderData.level) {
            // device end start pulse
            // expect data low
            state = WAIT_FOR_DEVICE_DATA_LOW;
            ESP_LOGV(TAG, "device start high");
        } else {
            // device begin start pulse
            // expect end start pulse
            state = WAIT_FOR_DEVICE_START;
            ESP_LOGV(TAG, "device start low");
        }
    } else if (state == WAIT_FOR_DEVICE_DATA_LOW) {
        if (decoderData.level) {
            // assume device end start pulse
            // expect data low
            state = WAIT_FOR_DEVICE_DATA_LOW;
            ESP_LOGV(TAG, "expected device data low, bit:%02d", bit);
        } else {
            // start data low period
            // expect data high period
            state = WAIT_FOR_DEVICE_DATA_HIGH;
            if (bit == 40) {
                // end of start pulse
                // not concluding a high period
                ESP_LOGV(TAG, "device data start");
            } else {
                // end of bit high duration
                // not correcting for wrapping after ~300000y power-on-time
                highDuration = decoderData.timestamp - previousTimestamp;
                // calculate bit value (1: high duration longer than low duration) and store
                bool bit_value = highDuration > lowDuration;
                if (bit_value) {
                    // data started as zeros, mark only the ones
                    data |= 1ULL << bit;
                }
                ESP_LOGV(TAG, "device data, bit:%02d, high:%d, value:%d", bit, highDuration, bit_value);
                if (bit == 0) {
                    state = WAIT_FOR_DEVICE_RELEASE;
                    frame_finished(data, decoderData.timestamp);
                }
            }
            // next bit
            bit--;
        }
    } else if (state == WAIT_FOR_DEVICE_DATA_HIGH) {
        if (decoderData.level) {
            lowDuration = decoderData.timestamp - previousTimestamp;
            state = WAIT_FOR_DEVICE_DATA_LOW;
            ESP_LOGV(TAG, "device data, bit:%02d, low:%d", bit, lowDuration);
        } else {
            fire_recoverable(decoderData.timestamp);
            ESP_LOGW(TAG, "bus error, expected high, bit:%02d", bit);
            one_wire_float();
        }
    } else if (state == WAIT_FOR_DEVICE_RELEASE) {
        if (decoderData.level) {
            state = HOST_BUS_FLOAT;
            ESP_LOGV(TAG, "device release");
        } else {
            // measurement result already reported
            state = HOST_BUS_FLOAT;
            ESP_LOGW(TAG, "bus error, expected device release");
        }
        one_wire_float();
    } else if (state == HOST_BUS_FLOAT) {
        // set to output causes edge detection
        if (decoderData.level) {
            ESP_LOGW(TAG, "bus error, expected host bus float");
        } else {
            ESP_LOGV(TAG, "host bus float");
        }
        state = WAIT_FOR_DEVICE_RATE_LIMIT;
    } else if (state == WAIT_FOR_DEVICE_RATE_LIMIT) {
        // wait for device hold-off (timeout on queue)
        // no edges expected while waiting
        // measurement result already reported
        ESP_LOGW(TAG, "bus error, while wait for device rate limit");
    } else if (state == COMPONENT_READY) {
        // waiting for start measurement instruction
        // no edges expected while waiting
        // measurement result already reported
        ESP_LOGW(TAG, "bus error, while ready");
    } else if (state == COMPONENT_RECOVERABLE) {
        // wait for device hold-off (timeout on queue)
        // ignore all edges until then
    } else if (state == COMPONENT_FATAL) {
        // bus alive while not setup correctly
        // ignore everything that follows
    } else {
        ESP_LOGE(TAG, "unknown state:%d, level:%d", state, decoderData.level);
    }
    previousTimestamp = decoderData.timestamp;
}

/**
 * Run forever.
 * Monitor decoder queue for ISR edge detection data and instructions.
 */
void IRAM_ATTR AM2301::run()
{
    while (true) {
        if (xQueueReceive(decoderQueue, &decoderData,
                MINIMUM_MEASUREMENT_INTERVAL_TICKS)) {
            if (decoderData.instruction == INSTRUCTION_EDGE_DETECTED) {
                handle_instruction_edge_detected();
            } else if (decoderData.instruction == INSTRUCTION_START) {
                handle_instruction_start();
            } else {
                ESP_LOGE(TAG, "run, unknown instruction:%d", decoderData.instruction);
            }
        } else {
            // timeout, no activity
            state = COMPONENT_READY;
        }
        vPortYield();
    }
}

/**
 * Handle finished frame.
 */
void AM2301::frame_finished(int64_t frame, int64_t timestamp)
{
    uint8_t b4 = frame >> 32;
    uint8_t b3 = frame >> 24;
    uint8_t b2 = frame >> 16;
    uint8_t b1 = frame >> 8;
    uint8_t b0 = frame;
    uint8_t actualChecksum = b0;
    uint8_t expectedChecksum = b4 + b3 + b2 + b1;
    if (actualChecksum == expectedChecksum) {
        // frame contains humidity times ten
        double humidity = ((frame >> 24) & 0xFFFF) / 10.0;
        // frame contains temperature times ten
        double t = ((frame >> 8) & 0x7FFF) / 10.0;
        // frame contains temperature sign as bit
        bool t_negative = (frame >> 8) & 0x8000;
        if (t_negative) {
            t *= -1.0;
        }
        double temperature = t + TEMPERATURE_C_TO_K;

        ESP_LOGD(TAG, "frame_finished, T:%.1fK, RH:%.1f%%", temperature, humidity);

        pubsub_publish_double(temperature_topic, temperature);
        pubsub_publish_double(humidity_topic, humidity);
        pubsub_publish_int(timestamp_topic, timestamp);
        pubsub_publish_int(status_topic, RESULT_OK);

    } else {
        fire_recoverable(timestamp);
        ESP_LOGW(TAG, "frame_finished, invalid checksum:%02X%02X%02X%02X%02X", b4, b3, b2, b1, b0);
        one_wire_float();
    }
}

/**
 * Report recoverable error as measurement result (once).
 */
void AM2301::fire_recoverable(int64_t timestamp)
{
    if (state != COMPONENT_RECOVERABLE) {
        state = COMPONENT_RECOVERABLE;

        pubsub_publish_int(status_topic, RESULT_RECOVERABLE);
    }
}

/**
 * Link C static world to C++ instance
 */
void IRAM_ATTR AM2301::task(void *pvParameter)
{
    if (pvParameter == 0) {
        ESP_LOGE(TAG, "task, invalid parameter (FATAL)");
    } else {
        // should be an instance
        AM2301 *pInstance = (AM2301*) pvParameter;
        pInstance->run();
    }
}

/**
 * ISR short and in IRAM.
 */
void IRAM_ATTR AM2301::isr_handler(void *pvParameter)
{
    AM2301 *pInstance = (AM2301*) pvParameter;
    AM2301::decoder_data_t data;
    data.instruction = INSTRUCTION_EDGE_DETECTED;
    data.timestamp = esp_timer_get_time();
    data.level = gpio_get_level(pInstance->pin);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(pInstance->decoderQueue, &data, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}
