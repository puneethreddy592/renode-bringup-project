#define DT_DRV_COMPAT zephyr_event_counter

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/irq.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/logging/log.h>
#include "event_counter.h"

LOG_MODULE_REGISTER(event_counter, CONFIG_EVENT_COUNTER_LOG_LEVEL);

#define REG_CTRL         0x00
#define REG_STATUS       0x04
#define REG_EVENT_COUNT  0x08
#define REG_TRIGGER      0x0C

struct event_counter_config {
	mm_reg_t base;
	void (*irq_config_func)(const struct device *dev);
};

struct event_counter_data {
	struct k_sem event_sem;
};

static inline uint32_t ec_read(const struct device *dev, uint32_t offset)
{
	const struct event_counter_config *cfg = dev->config;
	return sys_read32(cfg->base + offset);
}

static inline void ec_write(const struct device *dev, uint32_t offset, uint32_t val)
{
	const struct event_counter_config *cfg = dev->config;
	sys_write32(val, cfg->base + offset);
}

void event_counter_enable(const struct device *dev)
{
	ec_write(dev, REG_CTRL, 1);
}

void event_counter_disable(const struct device *dev)
{
	ec_write(dev, REG_CTRL, 0);
}

void event_counter_trigger(const struct device *dev)
{
	ec_write(dev, REG_TRIGGER, 0xFF);
}

uint32_t event_counter_get_count(const struct device *dev)
{
	return ec_read(dev, REG_EVENT_COUNT);
}

uint32_t event_counter_get_status(const struct device *dev)
{
	return ec_read(dev, REG_STATUS);
}

void event_counter_clear_status(const struct device *dev)
{
	ec_write(dev, REG_STATUS, 1);
}

int event_counter_wait_for_event(const struct device *dev, k_timeout_t timeout)
{
	struct event_counter_data *data = dev->data;
	return k_sem_take(&data->event_sem, timeout);
}

static void event_counter_isr(const struct device *dev)
{
	struct event_counter_data *data = dev->data;
	uint32_t status = event_counter_get_status(dev);

	LOG_INF("ISR fired, status=0x%x", status);

	if (status & 0x1) {
		event_counter_clear_status(dev);
		k_sem_give(&data->event_sem);
	}
}

static int event_counter_init(const struct device *dev)
{
	const struct event_counter_config *cfg = dev->config;
	struct event_counter_data *data = dev->data;

	k_sem_init(&data->event_sem, 0, 1);

	cfg->irq_config_func(dev);

	LOG_INF("Event counter driver initialized at 0x%lx", (unsigned long)cfg->base);
	return 0;
}

#define EVENT_COUNTER_INIT(inst) \
	static void event_counter_irq_config_##inst(const struct device *dev) \
	{ \
		IRQ_CONNECT(DT_INST_IRQN(inst), \
			    DT_INST_IRQ(inst, priority), \
			    event_counter_isr, \
			    DEVICE_DT_INST_GET(inst), \
			    0); \
		irq_enable(DT_INST_IRQN(inst)); \
	} \
	static const struct event_counter_config event_counter_cfg_##inst = { \
		.base = DT_INST_REG_ADDR(inst), \
		.irq_config_func = event_counter_irq_config_##inst, \
	}; \
	static struct event_counter_data event_counter_data_##inst; \
	DEVICE_DT_INST_DEFINE(inst, event_counter_init, NULL, \
			      &event_counter_data_##inst, \
			      &event_counter_cfg_##inst, POST_KERNEL, \
			      CONFIG_EVENT_COUNTER_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(EVENT_COUNTER_INIT)
