#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>
#include "event_counter.h"

#define EC_NODE DT_NODELABEL(eventcounter0)

int main(void)
{
	const struct device *ec = DEVICE_DT_GET(EC_NODE);

	if (!device_is_ready(ec)) {
		printk("Event counter device not ready!\n");
		return 0;
	}

	printk("Event counter device ready.\n");

	event_counter_enable(ec);
	printk("Enabled. Status=%u Count=%u\n",
	       event_counter_get_status(ec), event_counter_get_count(ec));

	event_counter_trigger(ec);
	printk("After trigger: Status=%u Count=%u\n",
	       event_counter_get_status(ec), event_counter_get_count(ec));

	event_counter_clear_status(ec);
	printk("After clear: Status=%u Count=%u\n",
	       event_counter_get_status(ec), event_counter_get_count(ec));

	return 0;
}
