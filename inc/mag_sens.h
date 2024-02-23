/**
* @file mag_sens.h

* @brief Description in .c
*
* @par 
*
* COPYRIGHT NOTICE: 
*/ 
#ifndef __MAG_SENS_H__
#define __MAG_SENS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ------> INCLUDES <------ */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

/* ------> MACROS <------ */

/* ------> DATA TYPES <------ */

/* ------> PUBLIC FUNCTION PROTOTYPES <------ */
class mag_sens {
public:
    // Constructor
    mag_sens(const struct gpio_dt_spec *input);

    // Destructor
    ~mag_sens();

    // Read the state of the switch (true for ON, false for OFF)
    bool readSwitchState();

    void read_wheel_data(uint32_t * wheel_ts, uint32_t * wheel_rev);

    void reset_wheel_data();

    // Perform any cleanup or resource release
    void cleanup();

    void enableSwitchInterrupt();
    
    void disableSwitchInterrupt();

private:
    struct k_mutex mag_sens_mutex;
    volatile uint32_t last_wheel_rev_ts;
    volatile uint32_t cum_wheel_rev;

    const struct gpio_dt_spec *mag_sw;
    struct gpio_callback switch_cb_data;

    // Initialize the magnetic sensor
    bool init();

    uint32_t last_irq_time = 0;
    // Private callback function for GPIO interrupt
    void switch_int_hndl(const struct device *port,
                         struct gpio_callback *cb,
                         gpio_port_pins_t pins);

    // Static wrapper function for interrupt handler
    static void switch_int_hndl_wrapper(const struct device *port,
                                        struct gpio_callback *cb,
                                        gpio_port_pins_t pins);
    
    struct k_work_delayable switch_debounce_work;
    void debounce_handler(struct k_work *work);
    static void debounce_handler_wrapper(struct k_work *work);
};



#ifdef __cplusplus
}
#endif

#endif // __MAG_SENS_H__