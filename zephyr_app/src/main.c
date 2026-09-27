#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/sys/printk.h>
#include "event_counter.h"

#define EC_NODE DT_NODELABEL(eventcounter0)
#define WDT_NODE DT_NODELABEL(iwdg)

static int setup_watchdog(const struct device *wdt_dev)
{
	struct wdt_timeout_cfg wdt_config = {
		.window.min = 0,
		.window.max = 2000, /* 2 second timeout */
		.callback = NULL,
		.flags = WDT_FLAG_RESET_SOC,
	};

	int wdt_channel_id = wdt_install_timeout(wdt_dev, &wdt_config);

	if (wdt_channel_id < 0) {
		printk("Watchdog install error: %d\n", wdt_channel_id);
		return wdt_channel_id;
	}

	int ret = wdt_setup(wdt_dev, 0);

	if (ret < 0) {
		printk("Watchdog setup error: %d\n", ret);
		return ret;
	}

	printk("Watchdog configured, channel %d\n", wdt_channel_id);
	return wdt_channel_id;
}

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

	printk("Triggering event and waiting for ISR...\n");
	event_counter_trigger(ec);

	int ret = event_counter_wait_for_event(ec, K_SECONDS(2));

	if (ret == 0) {
		printk("ISR fired! Status=%u Count=%u\n",
		       event_counter_get_status(ec), event_counter_get_count(ec));
	} else {
		printk("Timed out waiting for interrupt!\n");
	}

	/* --- Watchdog fault-recovery demo --- */
	const struct device *wdt_dev = DEVICE_DT_GET(WDT_NODE);

	if (!device_is_ready(wdt_dev)) {
		printk("Watchdog device not ready!\n");
		return 0;
	}

	int wdt_channel_id = setup_watchdog(wdt_dev);

	if (wdt_channel_id < 0) {
		return 0;
	}

	for (int i = 0; i < 3; i++) {
		printk("Feeding watchdog (iteration %d)\n", i);
		wdt_feed(wdt_dev, wdt_channel_id);
		k_sleep(K_MSEC(800));
	}

	printk("Deliberately NOT feeding watchdog now, expect reset...\n");
	k_sleep(K_SECONDS(3));

	/* Should never reach here if watchdog reset the SoC as expected */
	printk("ERROR: watchdog did not reset the system!\n");

	return 0;
}
