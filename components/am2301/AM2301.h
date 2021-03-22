// The author disclaims copyright to this source code.
#ifndef _AM2301_H_
#define _AM2301_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "math.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "pubsub.h"

/**
 * AM2301 temperature and relative humidity sensor.
 * Connected through one wire serial bus.
 */
class AM2301 {

public:
	AM2301();
	virtual ~AM2301();
	/**
	 * Setup once before use.
	 * @param pin one wire (input/output) pin
	 * @param temperature_topic temperature measurement topic [double, K].
	 * @param humidity_topic humidity measurement topic [double, %].
	 * @param status_topic measurement status topic [int, model_component_status_t].
	 * @param timestamp_topic measurement timestamp topic [int, timt_t].
	 * @param measurement_period_ms measurement period [ms].
	 */
	void setup(gpio_num_t pin,
			pubsub_topic_t temperature_topic,
			pubsub_topic_t humidity_topic,
			pubsub_topic_t status_topic,
			pubsub_topic_t timestamp_topic,
			uint32_t measurement_period_ms);

	typedef enum {
		RESULT_OK = 0,
		//
		RESULT_RECOVERABLE,
		//
		RESULT_FATAL,
	} result_status_t;

	static constexpr int MINIMUM_MEASUREMENT_PERIOD_MS = 2000;

private:
	/**
	 * Decoder instruction type
	 */
	typedef enum {
		INSTRUCTION_EDGE_DETECTED,
		INSTRUCTION_START,
	} decoder_instruction_t;

	/**
	 * Decoder data type
	 */
	typedef struct {
		/** instruction */
		decoder_instruction_t instruction;
		/** timestamp for INSTRUCTION_EDGE_DETECTED */
		int64_t timestamp;
		/** wire level for INSTRUCTION_EDGE_DETECTED */
		uint8_t level;
	} decoder_data_t;

	/**
	 * Component state
	 */
	typedef enum {
		COMPONENT_UNINITIALIZED = 0,
		COMPONENT_READY,
		HOST_SEND_START,
		WAIT_FOR_DEVICE_START,
		WAIT_FOR_DEVICE_DATA_LOW,
		WAIT_FOR_DEVICE_DATA_HIGH,
		WAIT_FOR_DEVICE_RELEASE,
		HOST_BUS_FLOAT,
		WAIT_FOR_DEVICE_RATE_LIMIT,
		COMPONENT_RECOVERABLE,
		COMPONENT_FATAL,
	} component_state_t;

	/**
	 * GPIO pin connected to AM2301 sensor
	 */
	gpio_num_t pin = GPIO_NUM_NC;

	uint32_t measurement_period_ms = MINIMUM_MEASUREMENT_PERIOD_MS;
	/**
	 * Queue to receive
	 * - ISR edge detection data, and
	 * - decoder instructions
	 */
	xQueueHandle decoderQueue = 0;

	/**
	 * Temperature measurement topic.
	 */
	pubsub_topic_t temperature_topic = 0;
	/**
	 * Humidity measurement topic.
	 */
	pubsub_topic_t humidity_topic = 0;
	/**
	 * Measurement status topic.
	 */
	pubsub_topic_t status_topic = 0;
	/**
	 * Measurement timestamp topic.
	 */
	pubsub_topic_t timestamp_topic = 0;

	// timestamp of previous edge detected
	int64_t previousTimestamp = 0;
	// collected data bits
	int64_t data = 0;
	// expected bit number, counting downwards, receiving msb first
	int8_t bit = 0;
	// duration of bit low,
	// used to calculate bit value when high duration is known
	int32_t lowDuration = 0;
	// duration of bit high,
	// used to calculate bit value when both low and high duration are known
	int32_t highDuration = 0;
	// data read from decoder queue
	AM2301::decoder_data_t decoderData;
	// component state
	component_state_t state = COMPONENT_UNINITIALIZED;

	void run();
	bool queue_instruction_start();
	void frame_finished(int64_t frame, int64_t timestamp);
	void fire_recoverable(int64_t timestamp);
	void handle_instruction_edge_detected();
	void handle_instruction_start();

	void one_wire_float();
	void one_wire_start();
	void one_wire_listen();

	static constexpr float TEMPERATURE_C_TO_K = 273.15;
	static constexpr int NUMBER_OF_EDGES_IN_DATA_FRAME = 100;
	static constexpr int MICRO_PER_MILLI = 1000;

	static void IRAM_ATTR task(void *pvParameter);
	static void IRAM_ATTR isr_handler(void *pvParameter);
};

#endif /* _AM2301_H_ */
