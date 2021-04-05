// The author disclaims copyright to this source code.

#ifndef _NVS_H_
#define _NVS_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "pubsub.h"

/**
 * Non-Volatile-Storage
 *
 * Reduce nvs write actions by coalescing changes during hold off period.
 * Only writing the last value for each topic update received.
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
    /** nvs namespace to group the key-value pairs */
    const char *ns = 0;
    /** hold off period in timer ticks */
    uint32_t hold_off_period_ticks = 0;
    /** one queue receiving all monitored topics */
    QueueHandle_t queue = 0;

    /**
     * list of topics (keys in nvs) that are monitored
     * (matches the number and order of the values)
     */
    const char **keys = 0;
    /**
     * list of messages (values in nvs) that are monitored
     * (matches the number and order of the keys)
     */
    pubsub_message_t **values = 0;
    /** the number of keys (and values) monitored */
    uint16_t number_of_keys = 0;

    void run();

    static void task(void *pvParameter);
};

#endif /* _NVS_H_ */
