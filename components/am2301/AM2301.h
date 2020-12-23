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
	 * @param resultQueue measurement result queue.
	 */
	void setup(gpio_num_t pin, xQueueHandle resultQueue);
	void measure();

	typedef enum {
		OK = 0,
		//
		RECOVERABLE,
		//
		FATAL,
	} error_t;

	/**
	 * Measurement result message.
	 */
	typedef struct {
		/**
		 * Overall result status.
		 */
		error_t result = OK;
		/**
		 * Temperature [K] of last succesful measurement
		 * NAN if none available
		 */
		float temperature = NAN;
		/**
		 * Relative humidity [%] of last succesful measurement
		 * NAN if none available
		 */
		float humidity = NAN;
		/**
		 * Timestamp of last succesful measurement
		 * INT64_MIN if none available
		 */
		int64_t timestamp = INT64_MIN;
	} result_t;

private:
	/**
	 * ISR instruction
	 */
	typedef enum {
		INSTRUCTION_EDGE_DETECTED,
		INSTRUCTION_START,
	} isr_instruction_t;

	/**
	 * ISR edge detection data
	 */
	typedef struct {
		isr_instruction_t instruction;
		int64_t timestamp;
		uint8_t level;
	} isr_data_t;

	/**
	 * Single wire bus state
	 */
	typedef enum {
		BUS_IDLE = 0,
		HOST_SEND_START,
		WAIT_FOR_DEVICE_START_LOW,
		WAIT_FOR_DEVICE_START_HIGH,
		WAIT_FOR_DEVICE_DATA_LOW,
		WAIT_FOR_DEVICE_DATA_HIGH,
		WAIT_FOR_DEVICE_RELEASE,
		BUS_ERROR
	} bus_state_t;

	/**
	 * Component state.
	 */
	error_t componentState = OK;
	/**
	 * Temperature [K]
	 * NAN if none
	 */
	float lastValidTemperature = NAN;
	/**
	 * Relative humidity [%]
	 * NAN if none
	 */
	float lastValidHumidity = NAN;

	/**
	 * Timestamp of last succesful measurement
	 * INT64_MIN if none
	 */
	int64_t lastValidTimestamp = INT64_MIN;

	/**
	 * GPIO pin connected to AM2301 sensor
	 */
	gpio_num_t pin = GPIO_NUM_NC;

	/**
	 * Queue to receive ISR edge detection data
	 */
	xQueueHandle queue = 0;

	/**
	 * Queue to send measurement results
	 */
	xQueueHandle resultQueue = 0;

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
	// used to calculate bit value when high duration is known
	int32_t highDuration = 0;
	// data read from ISR queue
	AM2301::isr_data_t isrData;
	// bus state
	bus_state_t state = BUS_IDLE;

	void run();
	void queue_instruction_start();
	void frame_finished(int64_t frame, int64_t timestamp);
	void fire_result(bool valid);

	void handle_instruction_edge_detected();
	void handle_instruction_start();

	void one_wire_float();
	void one_wire_start();
	void one_wire_listen();

	static constexpr float TEMPERATURE_C_TO_K = -273.15;
	static constexpr int NUMBER_OF_EDGES_IN_DATA_FRAME = 100;
	static constexpr uint64_t QUEUE_START_NEW_FRAME = -1;
	static constexpr uint32_t MINIMUM_MEASUREMENT_INTERVAL_MS = 5000;
	static constexpr int MICRO_PER_MILLI = 1000;

	static void IRAM_ATTR task(void *pvParameter);
	static void IRAM_ATTR isr_handler(void *pvParameter);
};

#endif /* _AM2301_H_ */
