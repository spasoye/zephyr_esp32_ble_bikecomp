#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include "../inc/mag_sens.h"
#include "../inc/ble_csc_app.h"

// #define SLEEP_TIME_MS	500

// #define SW0_NODE	DT_ALIAS(magsw)
// #if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
// #error "Unsupported board: sw0 devicetree alias is not defined"
// #endif

// /*
//  * Get button configuration from the devicetree sw0 alias. This is mandatory.
//  */
// static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
// 							      {0});
// static struct gpio_callback button_cb_data;

// /*
//  * The led0 devicetree alias is optional. If present, we'll use it
//  * to turn on the LED whenever the button is pressed.
//  */
// static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios,
// 						     {0});

// void button_pressed(const struct device *dev, struct gpio_callback *cb,
// 		    uint32_t pins)
// {
// 	// gpio_pin_interrupt_configure_dt(&button, GPIO_INT_DISABLE);
// 	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
// }

int main(void)
{
	int err;

	// Enable Bluetooth
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	// Callback when Bluetooth is ready
	bt_ready();

	while (1) {
		// Sleep for 1 second
		k_sleep(K_SECONDS(1));


		csc_simulation();


		// Simulate battery level notification
		bas_notify();
	}
	
	return 0;
}
