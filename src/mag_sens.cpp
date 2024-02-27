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
LOG_MODULE_REGISTER(mag_sens);

#define DEBOUNCE_TIMEOUT_MS 150
#define STACK_SIZE 2048
#define THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(threadStack, STACK_SIZE);

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
        LOG_INF("Failed to initialize magnetic sensor\n");
        // Handle initialization failure if needed
    }

    mag_sens_thread_id = k_thread_create(&mag_sens_thread_data, 
		threadStack,
		K_THREAD_STACK_SIZEOF(threadStack),
		mag_sens::static_event_hndl_thread,
		this, NULL, NULL,
		THREAD_PRIORITY, 0, K_FOREVER);

    k_sem_init(&mag_sens_sem,0, 1);
}

void mag_sens::sens_start()
{
    k_thread_start(mag_sens_thread_id);
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
bool mag_sens::read_wheel_data(uint32_t * wheel_ts, uint32_t * wheel_rev)
{
    bool ret=false;

    if (k_mutex_lock(&mag_sens_mutex, K_NO_WAIT) == 0)
    {
        *wheel_ts = last_wheel_rev_ts;
        *wheel_rev = cum_wheel_rev;
        k_mutex_unlock(&mag_sens_mutex);
        ret = true;
    }

    return ret;
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
    LOG_INF("Cleaning up magnetic sensor\n");
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
        LOG_INF("Error: button device %s is not ready\n",
               mag_sw->port->name);
        return false;
    }

    // Configure GPIO pin as input
    ret = gpio_pin_configure_dt(mag_sw, GPIO_INPUT);
    if (ret != 0) {
        LOG_INF("Error %d: failed to configure %s pin %d\n",
               ret, mag_sw->port->name, mag_sw->pin);
        return false;
    }

    // Configure GPIO interrupts
    ret = gpio_pin_interrupt_configure_dt(mag_sw,
                                          GPIO_INT_EDGE_TO_INACTIVE);
    if (ret != 0) {
        LOG_INF("Error %d: failed to configure interrupts on %s pin %d\n",
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

void mag_sens::event_hndl_thread()
{
    while (true)
    {
        k_sem_take(&mag_sens_sem, K_FOREVER);
        
        /* Lock the mutex before updating shared variables */
        k_mutex_lock(&mag_sens_mutex, K_FOREVER);

        /* Update shared variables */
        last_wheel_rev_ts = k_uptime_get_32();
        cum_wheel_rev++;

        LOG_INF("Event LWT: %d\n", last_wheel_rev_ts);
        LOG_INF("Event CWR: %d\n\n", cum_wheel_rev);

        /* Unlock the mutex */
        k_mutex_unlock(&mag_sens_mutex);
    }
    
}

void mag_sens::static_event_hndl_thread(void *object, void *, void *)
{
    auto threadObject = reinterpret_cast<mag_sens*>(object);
	threadObject->event_hndl_thread();
}

/**
 * @brief Enable switch interrupt.
 */
void mag_sens::enableSwitchInterrupt() {
    LOG_INF("Mag switch int enable\n");
    gpio_pin_interrupt_configure_dt(mag_sw,
                                    GPIO_INT_EDGE_TO_INACTIVE);
}

/**
 * @brief Disable switch interrupt.
 */
void mag_sens::disableSwitchInterrupt() {
    LOG_INF("Mag switch int disable\n");
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
    disableSwitchInterrupt();
    
    uint32_t curr_time = k_uptime_get_32();

    if ( (curr_time - last_irq_time) > DEBOUNCE_TIMEOUT_MS && (last_irq_time != 0))
    {
        // critical section
        k_sem_give(&mag_sens_sem);
    }

    last_irq_time = curr_time;
    // Schedule debounce work after a timeout
    k_work_schedule(&switch_debounce_work, K_MSEC(DEBOUNCE_TIMEOUT_MS));
}
