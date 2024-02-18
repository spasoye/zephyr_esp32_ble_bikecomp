/**
* @file ble_csc_app.h

* @brief Description in .c
*
* @par 
*
* COPYRIGHT NOTICE: 
*/ 
#ifndef __BLE_CSC_APP_H__
#define __BLE_CSC_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ------> INCLUDES <------ */
#include <stdbool.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/bas.h>

/* ------> MACROS <------ */

// Supported locations for the Cycling Speed and Cadence (CSC) sensor
#define CSC_SUPPORTED_LOCATIONS		{ CSC_LOC_OTHER, \
					  CSC_LOC_FRONT_WHEEL, \
					  CSC_LOC_REAR_WHEEL, \
					  CSC_LOC_LEFT_CRANK, \
					  CSC_LOC_RIGHT_CRANK }

// Features supported by the CSC sensor
#define CSC_FEATURE			(CSC_FEAT_WHEEL_REV | \
					 CSC_FEAT_CRANK_REV | \
					 CSC_FEAT_MULTI_SENSORS)

// CSC Sensor Locations
#define CSC_LOC_OTHER			0x00
#define CSC_LOC_TOP_OF_SHOE		0x01
#define CSC_LOC_IN_SHOE			0x02
#define CSC_LOC_HIP			0x03
#define CSC_LOC_FRONT_WHEEL		0x04
#define CSC_LOC_LEFT_CRANK		0x05
#define CSC_LOC_RIGHT_CRANK		0x06
#define CSC_LOC_LEFT_PEDAL		0x07
#define CSC_LOC_RIGHT_PEDAL		0x08
#define CSC_LOC_FRONT_HUB		0x09
#define CSC_LOC_REAR_DROPOUT		0x0a
#define CSC_LOC_CHAINSTAY		0x0b
#define CSC_LOC_REAR_WHEEL		0x0c
#define CSC_LOC_REAR_HUB		0x0d
#define CSC_LOC_CHEST			0x0e

// CSC Application error codes
#define CSC_ERR_IN_PROGRESS		0x80
#define CSC_ERR_CCC_CONFIG		0x81

// SC Control Point Opcodes
#define SC_CP_OP_SET_CWR		0x01
#define SC_CP_OP_CALIBRATION		0x02
#define SC_CP_OP_UPDATE_LOC		0x03
#define SC_CP_OP_REQ_SUPP_LOC		0x04
#define SC_CP_OP_RESPONSE		0x10

// SC Control Point Response Values
#define SC_CP_RSP_SUCCESS		0x01
#define SC_CP_RSP_OP_NOT_SUPP		0x02
#define SC_CP_RSP_INVAL_PARAM		0x03
#define SC_CP_RSP_FAILED		0x04

// CSC Feature
#define CSC_FEAT_WHEEL_REV		BIT(0)
#define CSC_FEAT_CRANK_REV		BIT(1)
#define CSC_FEAT_MULTI_SENSORS		BIT(2)

// CSC Measurement Flags
#define CSC_WHEEL_REV_DATA_PRESENT	BIT(0)
#define CSC_CRANK_REV_DATA_PRESENT	BIT(1)

/* ------> DATA TYPES <------ */

// Structure for write request to SC Control Point
struct write_sc_ctrl_point_req {
	uint8_t op;
	union {
		uint32_t cwr;
		uint8_t location;
	};
} __packed;

// Structure for SC Control Point indication
struct sc_ctrl_point_ind {
	uint8_t op;
	uint8_t req_op;
	uint8_t status;
	uint8_t data[];
} __packed;

// Structure for CSC Measurement notification
struct csc_measurement_nfy {
	uint8_t flags;
	uint8_t data[];
} __packed;

// Structure for Wheel Revolution data notification
struct wheel_rev_data_nfy {
	uint32_t cwr;
	uint16_t lwet;
} __packed;

// Structure for Crank Revolution data notification
struct crank_rev_data_nfy {
	uint16_t ccr;
	uint16_t lcet;
} __packed;

/* ------> PUBLIC FUNCTION PROTOTYPES <------ */
// Callback when Bluetooth is ready
void bt_ready(void);

// Battery level notification
void bas_notify(void);

// Function to simulate CSC data
void csc_simulation(void);

#ifdef __cplusplus
}
#endif

#endif // __BLE_CSC_APP_H__

