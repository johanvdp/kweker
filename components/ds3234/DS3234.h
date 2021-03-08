// The author disclaims copyright to this source code.
#ifndef _DS3234_H_
#define _DS3234_H_

#include "pubsub.h"
#include "freertos/timers.h"

/**
 * DS3234 time.
 */
class DS3234 {

public:
    DS3234();
    virtual ~DS3234();
    /**
     * Setup once before use.
     * @param timestamp_topic timestamp topic.
     */
    void setup(pubsub_topic_t timestamp_topic);

private:

    /**
     * Timestamp topic. Ticks since epoch.
     */
    pubsub_topic_t timestamp_topic = 0;

    void run();

    /** periodic timer task */
    static void task(TimerHandle_t xTimer);
};

#endif /* _DS3234_H_ */
