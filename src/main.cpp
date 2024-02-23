#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include "../inc/mag_sens.h"
#include "../inc/ble_csc_app.h"

#define SLEEP_TIME_MS	500

#define SW0_NODE	DT_ALIAS(magsw)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
							      {0});
static struct gpio_callback button_cb_data;

/*
 * The led0 devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios,
						     {0});

int main(void)
{
	int ret;

	// Enable Bluetooth
	ret = bt_enable(NULL);

	if (ret) {
		printk("Bluetooth init failed (err %d)\n", ret);
		return 0;
	}
	else
	{
		printk("Bluetooth initialized\n");
	}
	
	// Callback when Bluetooth is ready
	bt_ready();

	// Magnetic switch testing
	mag_sens a(&button);

	if (led.port && !gpio_is_ready_dt(&led)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led.port->name);
		led.port = NULL;
	}
	if (led.port) {
		ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led.port->name, led.pin);
			led.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led.port->name, led.pin);
		}
	}


	printk("TEST Press the button\n");

	if (led.port) {
		while (1) {
			static uint32_t wheel_rev_ts;
			static uint32_t cum_wheel_rev;
			// static bool val=true;
			// val = !val;
			// gpio_pin_set_dt(&led, val);
			// Sleep for 1 second

			k_sleep(K_SECONDS(1));
			a.read_wheel_data(&wheel_rev_ts, &cum_wheel_rev);
			printk("Last wheel timestamp: %d\n", wheel_rev_ts);
			printk("Cummulative wheel revolutions: %d\n\n", cum_wheel_rev);
			
			// csc_simulation();
			csc_send_attr(&cum_wheel_rev, &wheel_rev_ts, NULL, NULL, true, false);

			// Simulate battery level notification
			bas_notify();
		}
	}
}
