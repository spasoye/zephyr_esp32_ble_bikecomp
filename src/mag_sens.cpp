/**
* @file mag_sens.cpp
*
* @brief Implementation file for the mag_sens class.
*
* @par
*
* COPYRIGHT NOTICE:
*/
#include "../inc/mag_sens.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/irq.h>

/* ------> MACROS <------ */
#define DEBOUNCE_TIMEOUT_MS 200

/* ------> DATA TYPES <------ */

/* ------> PRIVATE FUNCTIONS PROTOTYPES <------ */

/* ------> STATIC DATA <------ */

/* ------> GLOBAL DATA <------ */

/* ------> PUBLIC FUNCTIONS <------ */
mag_sens::mag_sens(const struct gpio_dt_spec *input) {
    mag_sw = input;
    
    if (!init()) {
        printk("Failed to initialize magnetic sensor\n");
        // Handle initialization failure if needed
    }
}

bool mag_sens::readSwitchState()
{
    bool state;
    state = gpio_pin_get_dt(mag_sw);
    return state;
}

mag_sens::~mag_sens(){
    // Destructor
    return;
}

/* ------> PRIVATE FUNCTIONS <------ */
void mag_sens::cleanup() {
    // Perform any cleanup or resource release
    printk("Cleaning up magnetic sensor\n");
    // Add cleanup logic if needed
}

bool mag_sens::init() {
    int ret;

    if (!gpio_is_ready_dt(mag_sw)) {
        printk("Error: button device %s is not ready\n",
               mag_sw->port->name);
        return false;
    }

    ret = gpio_pin_configure_dt(mag_sw, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n",
               ret, mag_sw->port->name, mag_sw->pin);
        return false;
    }

    ret = gpio_pin_interrupt_configure_dt(mag_sw,
                                          GPIO_INT_EDGE_TO_INACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupts on %s pin %d\n",
            ret, mag_sw->port->name, mag_sw->pin);
        return false;
    }

    gpio_init_callback(&switch_cb_data, switch_int_hndl_wrapper, BIT(mag_sw->pin));
    gpio_add_callback(mag_sw->port, &switch_cb_data);

    k_work_init_delayable(&switch_debounce_work, debounce_handler_wrapper);

    return true;
}

void mag_sens::enableSwitchInterrupt() {
    printk("Mag switch int enable\n");
    gpio_pin_interrupt_configure_dt(mag_sw,
                                    GPIO_INT_EDGE_TO_INACTIVE);
}


void mag_sens::disableSwitchInterrupt() {
    printk("Mag switch int disable\n");
    gpio_pin_interrupt_configure_dt(mag_sw,
                                    GPIO_INT_DISABLE);
}

/* ------> INTERRUPT HANDLERS <------ */
/* Work handler function */
void mag_sens::debounce_handler(struct k_work *work) {
    printk("Debounce");

    enableSwitchInterrupt();
}

void mag_sens::debounce_handler_wrapper(struct k_work *work) {
    // Get the instance from the work item
    mag_sens* instance = CONTAINER_OF(work, mag_sens, switch_debounce_work);

    // Call the non-static member function
    instance->debounce_handler(work);
}

void mag_sens::switch_int_hndl(const struct device *port,
					struct gpio_callback *cb,
					gpio_port_pins_t pins)
{
    // Disable interrupts
    disableSwitchInterrupt();

    // Do something usefull
    printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());

    k_work_schedule(&switch_debounce_work, K_MSEC(DEBOUNCE_TIMEOUT_MS));
}

void mag_sens::switch_int_hndl_wrapper(const struct device *port,
                                         struct gpio_callback *cb,
                                         gpio_port_pins_t pins)
{
    // Get the instance from the callback data
    mag_sens *instance = CONTAINER_OF(cb, mag_sens, switch_cb_data);
    
    // Call the non-static member function
    instance->switch_int_hndl(port, cb, pins);
}