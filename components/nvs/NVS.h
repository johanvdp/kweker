// The author disclaims copyright to this source code.

#ifndef _NVS_H_
#define _NVS_H_

#include "nvs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pubsub.h"

/**
 * Non-Volatile-Storage
 *
 * - Load monitored topic values from NVS
 * - Monitor topics and save changed values to NVS
 *
 * Reduce nvs write actions by coalescing changes over hold off period.
 * Only writing the last value for each topic change received.
 *
 */
class NVS
{

public:
    NVS();
    virtual ~NVS();
    /**
     * Setup once before use.
     *
     * @param ns namespace containing key/value pairs
     * @param topic_list list of topics
     * @param number_of_topics number of topics
     * @param hold_off_period_ms hold of period in ms
     */
    void setup(const char *ns, const char *topic_list[], const size_t number_of_topics, const uint32_t hold_off_period_ms);

private:
    /** NVS namespace to group the key-value pairs */
    const char *ns = 0;
    /** Hold off period in timer ticks */
    uint32_t hold_off_period_ticks = 0;
    /** One queue receiving messages for all monitored topics */
    QueueHandle_t queue = 0;
    /**
     * List of messages (values in nvs) that are monitored
     */
    pubsub_message_t **messages = 0;
    /**
     * List of message changed flags.
     * (matches the number and order of the messages)
     */
    bool *changed = 0;
    /** The number of messages monitored */
    uint16_t number_of_messages = 0;

    /** NVS access handle */
    nvs_handle_t handle = 0;

    /**
     * Initialize message queue.
     * @return true if successful
     */
    bool init_queue();
    /**
     * Initialize topics.
     *
     * @param topic_list
     * @param number_of_topics
     * @return true if successful
     */
    bool init_topics(const char *topic_list[], const size_t number_of_topics);
    /**
     * subscribe to topics
     * @return true if successful
     */
    bool subscribe_topics();
    /**
     * Initialize non-volatile storage.
     * @return true if successful
     */
    bool init_nvs();
    /**
     * Check if change detected.
     * @return true if change detected
     */
    bool is_change_detected();

    /**
     * read all topics from nvs.
     */
    void read_nvs();

    /**
     * write all changed topics to nvs.
     */
    void write_nvs();

    /**
     * FreeRTOS task in instance context.
     */
    void run();

    /**
     * FreeRTOS task.
     * Link C static world to C++ world.
     */
    static void task(void *pvParameter);
};

#endif /* _NVS_H_ */
