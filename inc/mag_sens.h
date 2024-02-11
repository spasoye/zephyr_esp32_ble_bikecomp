/**
* @file MagSense.h
*
* @brief Header file for the MagSense class.
*
* @par
*
* COPYRIGHT NOTICE:
*/
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
#define SW0_NODE	DT_ALIAS(magsw)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

/* ------> DATA TYPES <------ */

/* ------> PUBLIC FUNCTION PROTOTYPES <------ */
class mag_sense {
public:
    // Constructor
    mag_sense(const struct gpio_dt_spec *input);

    // Destructor
    ~mag_sense();

    // Read the state of the switch (true for ON, false for OFF)
    bool readSwitchState();

    // Perform any cleanup or resource release
    void cleanup();

    static void switch_int_hndl(const struct device *port,
					struct gpio_callback *cb,
					gpio_port_pins_t pins);

private:
    const struct gpio_dt_spec *mag_sw;
    struct gpio_callback switch_cb_data;

    // Initialize the magnetic sensor
    bool init();
};



#ifdef __cplusplus
}
#endif

#endif // __MAG_SENS_H__



