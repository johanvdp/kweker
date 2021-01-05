// The author disclaims copyright to this source code.
#include "AM2301.h"

static const char *tag = "AM2301";

AM2301::AM2301() {
}

AM2301::~AM2301() {
}

void AM2301::setup(gpio_num_t pin, xQueueHandle resultQueue) {
	ESP_LOGD(tag, "setup, pin: %d, queue: %s", pin, resultQueue ? "y" : "n");

	if (state != COMPONENT_UNINITIALIZED) {
		state = COMPONENT_FATAL;
		ESP_LOGE(tag, "setup only once (FATAL)");
		return;
	}

	if (pin < GPIO_NUM_0 || pin >= GPIO_NUM_MAX) {
		state = COMPONENT_FATAL;
		ESP_LOGE(tag, "setup requires GPIO pin number (FATAL)");
		return;
	}
	this->pin = pin;

	if (resultQueue == NULL) {
		state = COMPONENT_FATAL;
		ESP_LOGE(tag, "setup requires result queue handle (FATAL)");
		return;
	}
	this->resultQueue = resultQueue;

	decoderQueue = xQueueCreate(NUMBER_OF_EDGES_IN_DATA_FRAME,
			sizeof(decoder_data_t));
	if (decoderQueue == 0) {
		state = COMPONENT_FATAL;
		ESP_LOGE(tag, "setup xQueueCreate failed (FATAL)");
		return;
	}

	BaseType_t ret = xTaskCreatePinnedToCore(&task, "setup", 2048, this,
			(2 | portPRIVILEGE_BIT), NULL, 1);
	if (ret != pdPASS) {
		state = COMPONENT_FATAL;
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
		state = COMPONENT_FATAL;
		ESP_LOGE(tag, "setup gpio_config failed: %d (FATAL)", ret);
		return;
	}

	ret = gpio_install_isr_service(
	ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM);
	if (ret != ESP_OK) {
		state = COMPONENT_FATAL;
		ESP_LOGE(tag, "setup gpio_install_isr_service failed: %d (FATAL)", ret);
		return;
	}

	ret = gpio_isr_handler_add(pin, isr_handler, this);
	if (ret != ESP_OK) {
		state = COMPONENT_FATAL;
		ESP_LOGE(tag, "setup gpio_isr_handler_add failed: %d (FATAL)", ret);
		return;
	}

	state = COMPONENT_READY;
}

bool AM2301::measure() {
	ESP_LOGD(tag, "measure");

	if (state == COMPONENT_FATAL) {
		ESP_LOGE(tag, "measure component state (FATAL)");
		return false;
	} else if (state == COMPONENT_UNINITIALIZED) {
		ESP_LOGE(tag, "measure component not setup");
		return false;
	} else if (state != COMPONENT_READY) {
		ESP_LOGE(tag, "measure component not ready");
		return false;
	}

	return queue_instruction_start();
}

/**
 * Put INSTRUCTION_START into the queue.
 *
 * @return True if adding was succesful. False if not (queue full).
 */

bool AM2301::queue_instruction_start() {
	ESP_LOGD(tag, "queue_instruction_start");

	decoder_data_t data;
	data.instruction = INSTRUCTION_START;
	BaseType_t ret = xQueueSend(decoderQueue, &data, (TickType_t ) 0);
	if (ret != pdPASS) {
		// queue full, might recover
		ESP_LOGE(tag, "queue_instruction_start xQueueSend failed");
		return false;
	}

	return true;
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
		if (decoderData.level) {
			// out of sync
			// missed device begin start pulse
			// expect data low next
			state = WAIT_FOR_DEVICE_DATA_LOW;
			ESP_LOGD(tag, "missed device start low");
		} else {
			// device begin start pulse
			// expect end start pulse
			state = WAIT_FOR_DEVICE_START_HIGH;
			ESP_LOGD(tag, "device start low");
		}
	} else if (state == WAIT_FOR_DEVICE_START_HIGH) {
		if (decoderData.level) {
			// device end start pulse
			// expect data low
			state = WAIT_FOR_DEVICE_DATA_LOW;
			ESP_LOGD(tag, "device start high");
		} else {
			fire_recoverable(decoderData.timestamp);
			ESP_LOGE(tag, "expected device data high");
			one_wire_float();
		}
	} else if (state == WAIT_FOR_DEVICE_DATA_LOW) {
		if (decoderData.level) {
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
				highDuration = decoderData.timestamp - previousTimestamp;
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
			ESP_LOGD(tag, "device data, bit: %02d, low: %d", bit, lowDuration);
		} else {
			fire_recoverable(decoderData.timestamp);
			ESP_LOGE(tag, "bus error, expected high, bit: %02d", bit);
			one_wire_float();
		}
	} else if (state == WAIT_FOR_DEVICE_RELEASE) {
		if (decoderData.level) {
			state = HOST_BUS_FLOAT;
			ESP_LOGD(tag, "device release");
		} else {
			// measurement result already reported
			state = HOST_BUS_FLOAT;
			ESP_LOGE(tag, "bus error, expected device release");
		}
		one_wire_float();
	} else if (state == HOST_BUS_FLOAT) {
		// set to output causes edge detection
		if (decoderData.level) {
			ESP_LOGE(tag, "bus error, expected host bus float");
		} else {
			ESP_LOGD(tag, "host bus float");
		}
		state = WAIT_FOR_DEVICE_RATE_LIMIT;
	} else if (state == WAIT_FOR_DEVICE_RATE_LIMIT) {
		// wait for device hold-off (timeout on queue)
		// no edges expected while waiting
		// measurement result already reported
		ESP_LOGE(tag, "bus error, while wait for device rate limit");
	} else if (state == COMPONENT_READY) {
		// waiting for start measurement instruction
		// no edges expected while waiting
		// measurement result already reported
		ESP_LOGE(tag, "bus error, while ready");
	} else if (state == COMPONENT_RECOVERABLE) {
		// wait for device hold-off (timeout on queue)
		// ignore all edges until then
	} else if (state == COMPONENT_FATAL) {
		// bus alive while not setup correctly
		// ignore everything that follows
	} else {
		ESP_LOGE(tag, "unknown state: %d, level: %d", state, decoderData.level);
	}
	previousTimestamp = decoderData.timestamp;
}

/**
 * Run forever.
 * Monitor decoder queue for ISR edge detection data and instructions.
 */
void IRAM_ATTR AM2301::run() {
	while (true) {
		if (xQueueReceive(decoderQueue, &decoderData,
				MINIMUM_MEASUREMENT_INTERVAL_TICKS)) {
			if (decoderData.instruction == INSTRUCTION_EDGE_DETECTED) {
				handle_instruction_edge_detected();
			} else if (decoderData.instruction == INSTRUCTION_START) {
				handle_instruction_start();
			} else {
				ESP_LOGE(tag, "run, unknown instruction: %d",
						decoderData.instruction);
			}
		} else {
			// timeout, no activity
			state = COMPONENT_READY;
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
		// frame contains humidity times ten
		float humidity = ((frame >> 24) & 0xFFFF) / 10.0;
		// frame contains temperature times ten
		float t = ((frame >> 8) & 0x7FFF) / 10.0;
		// frame contains temperature sign as bit
		bool t_negative = (frame >> 8) & 0x8000;
		if (t_negative) {
			t *= -1.0;
		}
		float temperature = t + TEMPERATURE_C_TO_K;

		ESP_LOGD(tag, "frame_finished, T: %.1fK, RH: %.1f%%", temperature,
				humidity);
		fire_result(RESULT_OK, temperature, humidity, timestamp);

	} else {
		ESP_LOGD(tag, "frame_finished, invalid checksum: %02X%02X%02X%02X%02X",
				b4, b3, b2, b1, b0);
		fire_recoverable(timestamp);
	}
}

/**
 * Fire result to result queue.
 */
void AM2301::fire_result(result_status_t status, float temperature,
		float humidity, int64_t timestamp) {
	result_t result;
	result.status = status;
	result.temperature = temperature;
	result.humidity = humidity;
	result.timestamp = timestamp;
	BaseType_t ret = xQueueSend(resultQueue, &result, 0);
	if (ret != pdPASS) {
		// queue full, might recover
		ESP_LOGE(tag, "fire_result queue full");
	}
}

/**
 * Report recoverable error as measurement result (once).
 */
void AM2301::fire_recoverable(int64_t timestamp) {
	if (state != COMPONENT_RECOVERABLE) {
		state = COMPONENT_RECOVERABLE;
		fire_result(RESULT_RECOVERABLE, NAN, NAN, timestamp);
	}
}

/**
 * Link C static world to C++ instance
 */
void IRAM_ATTR AM2301::task(void *pvParameter) {
	if (pvParameter == 0) {
		ESP_LOGE(tag, "task, invalid parameter (FATAL)");
	} else {
		// should be an instance
		AM2301 *pInstance = (AM2301*) pvParameter;
		pInstance->run();
	}
}

/**
 * ISR short and in IRAM.
 */
void IRAM_ATTR AM2301::isr_handler(void *pvParameter) {
	AM2301 *pInstance = (AM2301*) pvParameter;
	AM2301::decoder_data_t data;
	data.instruction = INSTRUCTION_EDGE_DETECTED;
	data.timestamp = esp_timer_get_time();
	data.level = gpio_get_level(pInstance->pin);
	xQueueSendFromISR(pInstance->decoderQueue, &data, NULL);
}
