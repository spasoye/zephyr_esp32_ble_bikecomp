/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

#define SLEEP_TIME_MS 500
#define SLEEP_FAST_TIME_MS 250

#define LED0_NODE DT_ALIAS(led0)
#define SW_NODE DT_ALIAS(sw0)

#define DEBOUNCE_TIMEOUT_MS 50

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec sw = GPIO_DT_SPEC_GET(SW_NODE, gpios);

bool fast = false;
struct gpio_callback sw_cb_data;

int period() 
{
	if (fast){
		return SLEEP_FAST_TIME_MS;
	} else {
		return SLEEP_TIME_MS;
	}
}

/* Work handler function */
void button_action_work_handler(struct k_work *work) {
    fast = !fast;
    LOG_INF("Pressing the button at %" PRIu32 "; period is %dms", k_cycle_get_32(), period());
}

/* Register the work handler */
K_WORK_DEFINE(button_action_work, button_action_work_handler);

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	k_work_submit(&button_action_work);
}

void main(void)
{
	int ret;

	LOG_INF("Starting demo");

	// led

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED not running");
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Bad LED Configuration");
		return;
	}

	// switch

	if (!gpio_is_ready_dt(&sw)) {
		LOG_ERR("SW not running");
		return;
	}

	ret = gpio_pin_configure_dt(&sw, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Bad SW Configuration");
		return;
	}

	// interrupts

	ret = gpio_pin_interrupt_configure_dt(&sw, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Bad INTR configuration");
		return;
	}

	// setup the button press callback
	gpio_init_callback(&sw_cb_data, button_pressed, BIT(sw.pin));
	gpio_add_callback(sw.port, &sw_cb_data);

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return;
		}
		k_msleep(period());
	}
}