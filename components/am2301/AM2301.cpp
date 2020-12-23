// The author disclaims copyright to this source code.
#include "AM2301.h"

static const char *tag = "AM2301";

AM2301::AM2301() {
}

AM2301::~AM2301() {
}

void AM2301::setup(gpio_num_t pin, xQueueHandle resultQueue) {
	ESP_LOGD(tag, "setup, pin: %d, queue: %s", pin, resultQueue ? "y" : "n");

	if (pin < GPIO_NUM_0 || pin >= GPIO_NUM_MAX) {
		componentState = FATAL;
		ESP_LOGE(tag, "setup requires GPIO pin number (FATAL)");
		return;
	}
	this->pin = pin;

	if (resultQueue == NULL) {
		componentState = FATAL;
		ESP_LOGE(tag, "setup requires result queue handle (FATAL)");
		return;
	}
	this->resultQueue = resultQueue;

	queue = xQueueCreate(NUMBER_OF_EDGES_IN_DATA_FRAME, sizeof(isr_data_t));
	if (queue == 0) {
		componentState = FATAL;
		ESP_LOGE(tag, "setup xQueueCreate failed (FATAL)");
		return;
	}

	BaseType_t ret = xTaskCreatePinnedToCore(&task, "setup", 2048, this,
			(2 | portPRIVILEGE_BIT), NULL, 1);
	if (ret != pdPASS) {
		componentState = FATAL;
		ESP_LOGE(tag, "setup xTaskCreate failed: %d (FATAL)", ret);
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
		componentState = FATAL;
		ESP_LOGE(tag, "setup gpio_config failed: %d (FATAL)", ret);
		return;
	}
	one_wire_listen();

	ret = gpio_install_isr_service(
	ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM);
	if (ret != ESP_OK) {
		componentState = FATAL;
		ESP_LOGE(tag, "setup gpio_install_isr_service failed: %d (FATAL)", ret);
		return;
	}

	ret = gpio_isr_handler_add(pin, isr_handler, this);
	if (ret != ESP_OK) {
		componentState = FATAL;
		ESP_LOGE(tag, "setup gpio_isr_handler_add failed: %d (FATAL)", ret);
		return;
	}
}

void AM2301::measure() {
	ESP_LOGD(tag, "measure");

	if (componentState == FATAL) {
		ESP_LOGE(tag, "measure component state (FATAL)");
		return;
	}

	queue_instruction_start();
}

void AM2301::queue_instruction_start() {
	ESP_LOGD(tag, "queue_instruction_start");

	isr_data_t data;
	data.instruction = INSTRUCTION_START;
	BaseType_t ret = xQueueSend(queue, &data, (TickType_t ) 0);
	if (ret != pdPASS) {
		// queue full, might recover
		ESP_LOGE(tag, "queue_instruction_start xQueueSend failed");
	}
}

/**
 * listen to one wire bus.
 * - pin as input with pull-up
 */
void AM2301::one_wire_listen() {
	gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
	gpio_set_direction(pin, GPIO_MODE_INPUT);
}

/**
 * float one wire bus with pull-up.
 * - pin as open drain output
 * - release bus
 */
void AM2301::one_wire_float() {
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
void AM2301::one_wire_start() {
	gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
	gpio_set_level(pin, 0);
	ets_delay_us(1000);
	gpio_set_level(pin, 1);
}

void AM2301::handle_instruction_start() {
	ESP_LOGD(tag, "handle_instruction_start");

	state = HOST_SEND_START;
	data = 0;
	bit = 40;
	one_wire_start();
	one_wire_listen();

	state = WAIT_FOR_DEVICE_START_LOW;
}

void AM2301::handle_instruction_edge_detected() {
	if (state == WAIT_FOR_DEVICE_START_LOW) {
		if (isrData.level) {
			// assume device end start pulse
			// expect data low
			state = WAIT_FOR_DEVICE_DATA_LOW;
			ESP_LOGD(tag, "expected device start low");
		} else {
			// device begin start pulse
			// expect end start pulse
			state = WAIT_FOR_DEVICE_START_HIGH;
			ESP_LOGD(tag, "device start low");
		}
	} else if (state == WAIT_FOR_DEVICE_START_HIGH) {
		if (isrData.level) {
			// device end start pulse
			// expect data low
			state = WAIT_FOR_DEVICE_DATA_LOW;
			ESP_LOGD(tag, "device start high");
		} else {
			state = BUS_ERROR;
			ESP_LOGE(tag, "expected device data high");
			one_wire_float();
		}
	} else if (state == WAIT_FOR_DEVICE_DATA_LOW) {
		if (isrData.level) {
			// assume device end start pulse
			// expect data low
			state = WAIT_FOR_DEVICE_DATA_LOW;
			ESP_LOGD(tag, "expected device data low, bit: %02d", bit);
		} else {
			// start data low period
			// expect data high period
			state = WAIT_FOR_DEVICE_DATA_HIGH;
			if (bit == 40) {
				// end of start pulse
				// not concluding a high period
				ESP_LOGD(tag, "device data start");
			} else {
				// end of bit high duration
				// not correcting for wrapping after ~300000y power-on-time
				highDuration = isrData.timestamp - previousTimestamp;
				// calculate bit value (1: high duration longer than low duration) and store
				bool bit_value = highDuration > lowDuration;
				if (bit_value) {
					// data started as zeros, mark only the ones
					data |= 1ULL << bit;
				}
				ESP_LOGD(tag, "device data, bit: %02d, high: %d, value: %d",
						bit, highDuration, bit_value);
				if (bit == 0) {
					state = WAIT_FOR_DEVICE_RELEASE;
					frame_finished(data, lastValidTimestamp);
				}
			}
			// next bit
			bit--;
		}
	} else if (state == WAIT_FOR_DEVICE_DATA_HIGH) {
		if (isrData.level) {
			lowDuration = isrData.timestamp - previousTimestamp;
			state = WAIT_FOR_DEVICE_DATA_LOW;
			ESP_LOGD(tag, "device data, bit: %02d, low: %d", bit, lowDuration);
		} else {
			state = BUS_ERROR;
			ESP_LOGE(tag, "framing error, expected high, bit: %02d", bit);
			one_wire_float();
		}
	} else if (state == WAIT_FOR_DEVICE_RELEASE) {
		if (isrData.level) {
			// concluding data transmission
			state = BUS_IDLE;
			ESP_LOGD(tag, "device release");
			one_wire_float();
		} else {
			state = BUS_ERROR;
			ESP_LOGE(tag, "framing error, expected device release");
			one_wire_float();
		}
	} else if (state == BUS_IDLE) {
		// ignore
	} else if (state == BUS_ERROR) {
		// ignore
	} else {
		ESP_LOGE(tag, "unknown state: %d, level: %d", state, isrData.level);
	}
	previousTimestamp = isrData.timestamp;
}

/**
 * Run forever.
 * Monitor ISR edge detection data and detect frames.
 */
void IRAM_ATTR AM2301::run() {
	while (true) {
		if (xQueueReceive(queue, &isrData, portMAX_DELAY)) {
			if (isrData.instruction == INSTRUCTION_EDGE_DETECTED) {
				handle_instruction_edge_detected();
			} else if (isrData.instruction == INSTRUCTION_START) {
				handle_instruction_start();
			} else {
				ESP_LOGE(tag, "run, unknown instruction: %d",
						isrData.instruction);
			}
		}
	}
}

/**
 * Handle finished frame.
 */
void AM2301::frame_finished(int64_t frame, int64_t timestamp) {
	uint8_t b4 = frame >> 32;
	uint8_t b3 = frame >> 24;
	uint8_t b2 = frame >> 16;
	uint8_t b1 = frame >> 8;
	uint8_t b0 = frame;
	uint8_t actualChecksum = b0;
	uint8_t expectedChecksum = b4 + b3 + b2 + b1;
	if (actualChecksum == expectedChecksum) {
		// store last valid measurement
		this->lastValidTimestamp = timestamp;
		// frame contains humidity times ten
		this->lastValidHumidity = ((frame >> 24) & 0xFFFF) / 10.0;
		// frame contains temperature times ten
		float t = ((frame >> 8) & 0x7FFF) / 10.0;
		// frame contains temperature sign as bit
		bool t_negative = (frame >> 8) & 0x8000;
		if (t_negative) {
			t *= -1.0;
		}
		this->lastValidTemperature = t - TEMPERATURE_C_TO_K;

		ESP_LOGD(tag, "frame_finished, T: %.1fK, RH: %.1f%%",
				lastValidTemperature, lastValidHumidity);
		fire_result(true);

	} else {
		ESP_LOGD(tag,
				"frame_finished, invalid checksum: %02X%02X%02X%02X%02X",
				b4, b3, b2, b1, b0);
		fire_result(false);
	}
}

/**
 * Fire result to result queue.
 */
void AM2301::fire_result(bool valid) {
	result_t result;
	result.result = valid ? OK : RECOVERABLE;
	result.temperature = this->lastValidTemperature;
	result.humidity = this->lastValidHumidity;
	BaseType_t ret = xQueueSend(resultQueue, &result, 0);
	if (ret != pdPASS) {
		// queue full, might recover
		ESP_LOGE(tag, "fire_result queue full");
	}
}

/**
 * Link C static world to C++ instance
 */
void AM2301::task(void *pvParameter) {
	if (pvParameter == 0) {
		ESP_LOGE(tag, "task, invalid parameter (FATAL)");
	} else {
		AM2301 *pInstance = (AM2301*) pvParameter;
		pInstance->run();
	}
}

/**
 * ISR short and in IRAM.
 */
void IRAM_ATTR AM2301::isr_handler(void *pvParameter) {
	AM2301 *pInstance = (AM2301*) pvParameter;
	AM2301::isr_data_t data;
	data.instruction = INSTRUCTION_EDGE_DETECTED;
	data.timestamp = esp_timer_get_time();
	data.level = gpio_get_level(pInstance->pin);
	xQueueSendFromISR(pInstance->queue, &data, NULL);
}
