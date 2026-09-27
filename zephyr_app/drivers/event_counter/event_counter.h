#ifndef EVENT_COUNTER_H_
#define EVENT_COUNTER_H_

#include <zephyr/device.h>

void event_counter_enable(const struct device *dev);
void event_counter_disable(const struct device *dev);
void event_counter_trigger(const struct device *dev);
uint32_t event_counter_get_count(const struct device *dev);
uint32_t event_counter_get_status(const struct device *dev);
void event_counter_clear_status(const struct device *dev);

#endif
