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

/* ------> MACROS <------ */
#define DEBOUNCE_TIMEOUT_MS 150

/* ------> DATA TYPES <------ */

/* ------> PRIVATE FUNCTIONS PROTOTYPES <------ */

/* ------> STATIC DATA <------ */

/* ------> GLOBAL DATA <------ */

/* ------> PUBLIC FUNCTIONS <------ */

/**
 * @brief Constructor for the MagSense class.
 *
 * @param input Pointer to the GPIO device specification.
 */
mag_sens::mag_sens(const struct gpio_dt_spec *input) {
    mag_sw = input;

    // Initialize magnetic sensor
    if (!init()) {
        printk("Failed to initialize magnetic sensor\n");
        // Handle initialization failure if needed
    }
}

/**
 * @brief Read the state of the switch.
 *
 * @return True if the switch is ON, false if OFF.
 */
bool mag_sens::readSwitchState() {
    bool state;
    state = gpio_pin_get_dt(mag_sw);
    return state;
}

/**
 * @brief Returns last revolution timestamp (ms) and cumulative wheel 
 *        revolutions.
 * 
 * @param wheel_ts Pointer to last wheel revolution timestamp variable.
 * @param cum_wheel_rev Pointer to total wheel revolution counter variable.
 */
void mag_sens::read_wheel_data(uint32_t * wheel_ts, uint32_t * wheel_rev)
{
    
    // if (k_mutex_lock(&mag_sens_mutex, K_NO_WAIT))
    // {
        *wheel_ts = last_wheel_rev_ts;
        *wheel_rev = cum_wheel_rev;
        k_mutex_unlock(&mag_sens_mutex);
    // }
}

void mag_sens::reset_wheel_data()
{
    if (k_mutex_lock(&mag_sens_mutex, K_NO_WAIT))
    {
        cum_wheel_rev = 0;
        k_mutex_unlock(&mag_sens_mutex);
    }
}

/**
 * @brief Destructor for the MagSense class.
 */
mag_sens::~mag_sens() {
    // Destructor TODO
    return;
}

/* ------> PRIVATE FUNCTIONS <------ */

/**
 * @brief Perform any cleanup or resource release.
 */
void mag_sens::cleanup() {
    // Perform any cleanup or resource release
    printk("Cleaning up magnetic sensor\n");
    // Add cleanup logic if needed
}

/**
 * @brief Initialize the magnetic sensor.
 *
 * @return True if initialization is successful, false otherwise.
 */
bool mag_sens::init() {
    int ret;

    // Check if GPIO device is ready
    if (!gpio_is_ready_dt(mag_sw)) {
        printk("Error: button device %s is not ready\n",
               mag_sw->port->name);
        return false;
    }

    // Configure GPIO pin as input
    ret = gpio_pin_configure_dt(mag_sw, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n",
               ret, mag_sw->port->name, mag_sw->pin);
        return false;
    }

    // Configure GPIO interrupts
    ret = gpio_pin_interrupt_configure_dt(mag_sw,
                                          GPIO_INT_EDGE_TO_INACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupts on %s pin %d\n",
            ret, mag_sw->port->name, mag_sw->pin);
        return false;
    }

    // Initialize GPIO callback and add it
    gpio_init_callback(&switch_cb_data, switch_int_hndl_wrapper, BIT(mag_sw->pin));
    gpio_add_callback(mag_sw->port, &switch_cb_data);

    // Initialize debounce work and return success
    k_work_init_delayable(&switch_debounce_work, debounce_handler_wrapper);

    // Initialize variables and lock mutex
    k_mutex_init(&mag_sens_mutex);
    last_wheel_rev_ts = 0;
    cum_wheel_rev = 0;
    return true;
}

/**
 * @brief Enable switch interrupt.
 */
void mag_sens::enableSwitchInterrupt() {
    printk("Mag switch int enable\n");
    gpio_pin_interrupt_configure_dt(mag_sw,
                                    GPIO_INT_EDGE_TO_INACTIVE);
}

/**
 * @brief Disable switch interrupt.
 */
void mag_sens::disableSwitchInterrupt() {
    printk("Mag switch int disable\n");
    gpio_pin_interrupt_configure_dt(mag_sw,
                                    GPIO_INT_DISABLE);
}

/* ------> INTERRUPT HANDLERS <------ */

/**
 * @brief Work handler function for switch debouncing.
 *
 * @param work Pointer to the k_work structure.
 */
void mag_sens::debounce_handler(struct k_work *work) {
    printk("Debounce");

    // Enable switch interrupt after debounce timeout
    enableSwitchInterrupt();
}

/**
 * @brief Wrapper for debounce_handler to use non-static member function.
 *
 * @param work Pointer to the k_work structure.
 */
void mag_sens::debounce_handler_wrapper(struct k_work *work) {
    // Get the instance from the work item
    mag_sens* instance = CONTAINER_OF(work, mag_sens, switch_debounce_work);

    // Call the non-static member function
    instance->debounce_handler(work);
}

/**
 * @brief Switch interrupt handler.
 *
 * @param port Pointer to the GPIO device structure.
 * @param cb Pointer to the GPIO callback structure.
 * @param pins GPIO port pins.
 */
void mag_sens::switch_int_hndl(const struct device *port,
                                 struct gpio_callback *cb,
                                 gpio_port_pins_t pins) {
    // Disable interrupts
    // disableSwitchInterrupt();
    
    uint32_t curr_time = k_uptime_get_32();

    if ( (curr_time - last_irq_time) > DEBOUNCE_TIMEOUT_MS && (last_irq_time != 0))
    {
        // TODO: critical section
        // if (k_mutex_lock(&mag_sens_mutex, K_NO_WAIT))
        // {
            last_wheel_rev_ts = curr_time;
            cum_wheel_rev = cum_wheel_rev + 1;
        //     k_mutex_unlock(&mag_sens_mutex);
        // }

        // Schedule debounce work after a timeout
        // k_work_schedule(&switch_debounce_work, K_MSEC(DEBOUNCE_TIMEOUT_MS));
    }

    last_irq_time = curr_time;
}

/**
 * @brief Wrapper for switch_int_hndl to use non-static member function.
 *
 * @param port Pointer to the GPIO device structure.
 * @param cb Pointer to the GPIO callback structure.
 * @param pins GPIO port pins.
 */
void mag_sens::switch_int_hndl_wrapper(const struct device *port,
                                        struct gpio_callback *cb,
                                        gpio_port_pins_t pins) {
    // Get the instance from the callback data
    mag_sens *instance = CONTAINER_OF(cb, mag_sens, switch_cb_data);

    // Call the non-static member function
    instance->switch_int_hndl(port, cb, pins);
}