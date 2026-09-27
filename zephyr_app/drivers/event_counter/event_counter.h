#ifndef EVENT_COUNTER_H_
#define EVENT_COUNTER_H_

#include <zephyr/device.h>
#include <zephyr/kernel.h>

void event_counter_enable(const struct device *dev);
void event_counter_disable(const struct device *dev);
void event_counter_trigger(const struct device *dev);
uint32_t event_counter_get_count(const struct device *dev);
uint32_t event_counter_get_status(const struct device *dev);
void event_counter_clear_status(const struct device *dev);

/* Blocks until the ISR signals an event, or timeout expires. Returns 0 on success. */
int event_counter_wait_for_event(const struct device *dev, k_timeout_t timeout);

#endif
