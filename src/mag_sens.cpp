/**
* @file MagSense.cpp
*
* @brief Implementation file for the MagSense class.
*
* @par
*
* COPYRIGHT NOTICE:
*/
#include "../inc/mag_sens.h"

/* ------> MACROS <------ */

/* ------> DATA TYPES <------ */

/* ------> PRIVATE FUNCTIONS PROTOTYPES <------ */

/* ------> STATIC DATA <------ */

/* ------> GLOBAL DATA <------ */

/* ------> PUBLIC FUNCTIONS <------ */
mag_sense::mag_sense(const struct gpio_dt_spec *input) {
    mag_sw = input;
    
    if (!init()) {
        printk("Failed to initialize magnetic sensor\n");
        // Handle initialization failure if needed
    }
}

bool mag_sense::readSwitchState()
{
    bool state;
    state = gpio_pin_get_dt(mag_sw);
    return state;
}

mag_sense::~mag_sense(){
    // Destructor
}

/* ------> PRIVATE FUNCTIONS <------ */
void mag_sense::switch_int_hndl(const struct device *port,
					struct gpio_callback *cb,
					gpio_port_pins_t pins)
{
    // gpio_pin_interrupt_configure_dt(&button, GPIO_INT_DISABLE);
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

void mag_sense::cleanup() {
    // Perform any cleanup or resource release
    printk("Cleaning up magnetic sensor\n");
    // Add cleanup logic if needed
}

bool mag_sense::init() {
    int ret;

    if (!gpio_is_ready_dt(mag_sw)) {
		printk("Error: button device %s is not ready\n",
		       mag_sw->port->name);
		return 0;
	}

	ret = gpio_pin_configure_dt(mag_sw, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, mag_sw->port->name, mag_sw->pin);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(mag_sw,
					      GPIO_INT_EDGE_TO_INACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupts on %s pin %d\n",
			ret, mag_sw->port->name, mag_sw->pin);
		return 0;
	}

	gpio_init_callback(&switch_cb_data, switch_int_hndl, BIT(mag_sw->pin));
	gpio_add_callback(mag_sw->port, &switch_cb_data);

    return true;
    // Call the init function during object construction
}

/* ------> INTERRUPT HANDLERS <------ */
