/**
* @file ble_csc_app.c

* @brief TODO
*
* @par TODO
*
* COPYRIGHT NOTICE: 
* 
*/
#include "../inc/ble_csc_app.h"
#include <zephyr/logging/log.h>

/* ------> MACROS <------ */
LOG_MODULE_REGISTER(ble_csc_app);

/* ------> DATA TYPES <------ */

/* ------> PRIVATE FUNCTIONS PROTOTYPES <------ */

// Callback when CCC (Client Characteristic Configuration) for CSC Measurement changes
static void csc_meas_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				     uint16_t value);

// Callback when CCC for SC Control Point changes
static void ctrl_point_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				       uint16_t value);

// Read callback for the Sensor Location characteristic
static ssize_t read_location(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr, void *buf,
			     uint16_t len, uint16_t offset);

// Read callback for the CSC Feature characteristic
static ssize_t read_csc_feature(struct bt_conn *conn,
				const struct bt_gatt_attr *attr, void *buf,
				uint16_t len, uint16_t offset);

// Function to send indication for SC Control Point
static void ctrl_point_ind(struct bt_conn *conn, uint8_t req_op, uint8_t status,
			   const void *data, uint16_t data_len);

// Write callback for SC Control Point
static ssize_t write_ctrl_point(struct bt_conn *conn,
				const struct bt_gatt_attr *attr,
				const void *buf, uint16_t len, uint16_t offset,
				uint8_t flags);

// Function to send CSC Measurement notification
static void measurement_nfy(struct bt_conn *conn, uint32_t cwr, uint16_t lwet,
			    uint16_t ccr, uint16_t lcet);

// Callback when a connection is established
static void connected(struct bt_conn *conn, uint8_t err);

// Callback when a connection is disconnected
static void disconnected(struct bt_conn *conn, uint8_t reason);
/* ------> STATIC DATA <------ */
// Cycling Speed and Cadence Service declaration

// Supported locations array
static uint8_t supported_locations[] = CSC_SUPPORTED_LOCATIONS;
// Current Sensor Location
static uint8_t sensor_location;

// Flag to simulate CSC data TODO
static bool csc_simulate;
// Flag indicating if the Control Point is configured
static bool ctrl_point_configured;

// Cumulative Wheel Revolutions
static uint32_t c_wheel_revs;
// Last Wheel Event Time
static uint16_t lwet;

// Cumulative Crank Revolutions
static uint16_t ccr;
// Last Crank Event Time
static uint16_t lcet;

// Advertising data
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_CSC_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL))
};

// Definition of the Cycling Speed and Cadence (CSC) GATT service
BT_GATT_SERVICE_DEFINE(csc_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_CSC),
	BT_GATT_CHARACTERISTIC(BT_UUID_CSC_MEASUREMENT, BT_GATT_CHRC_NOTIFY,
			       0x00, NULL, NULL, NULL),
	BT_GATT_CCC(csc_meas_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_SENSOR_LOCATION, BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, read_location, NULL,
			       &sensor_location),
	BT_GATT_CHARACTERISTIC(BT_UUID_CSC_FEATURE, BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, read_csc_feature, NULL, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_SC_CONTROL_POINT,
			       BT_GATT_CHRC_WRITE | BT_GATT_CHRC_INDICATE,
			       BT_GATT_PERM_WRITE, NULL, write_ctrl_point,
			       &sensor_location),
	BT_GATT_CCC(ctrl_point_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

// Connection callbacks
BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

/* ------> GLOBAL DATA <------ */

/* ------> PUBLIC FUNCTIONS <------ */

// Callback when Bluetooth is ready
void bt_ready(void)
{
	int err;

	LOG_INF("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_INF("Advertising failed to start (err %d)\n", err);
		return;
	}

	LOG_INF("Advertising successfully started\n");
}

// Battery level notification
void bas_notify(void)
{
	uint8_t battery_level = bt_bas_get_battery_level();

	battery_level--;

	if (!battery_level) {
		battery_level = 100U;
	}

	bt_bas_set_battery_level(battery_level);
}

// Function to simulate CSC data
void csc_simulation(void)
{
	static uint8_t i;
	uint32_t rand = sys_rand32_get();
	bool nfy_crank = false, nfy_wheel = false;

	/* Measurements don't have to be updated every second */
	if (!(i % 2)) {
		lwet += 1050 + rand % 50;
		c_wheel_revs += 2U;
		nfy_wheel = true;
	}

	if (!(i % 3)) {
		lcet += 1000 + rand % 50;
		ccr += 1U;
		nfy_crank = true;
	}

	/*
	 * In typical applications, the CSC Measurement characteristic is
	 * notified approximately once per second. This interval may vary
	 * and is determined by the Server and not required to be configurable
	 * by the Client.
	 */
	measurement_nfy(NULL, nfy_wheel ? c_wheel_revs : 0, nfy_wheel ? lwet : 0,
			nfy_crank ? ccr : 0, nfy_crank ? lcet : 0);

	/*
	 * The Last Crank Event Time value and Last Wheel Event Time roll over
	 * every 64 seconds.
	 */
	if (!(i % 64)) {
		lcet = 0U;
		lwet = 0U;
		i = 0U;
	}

	i++;
}

void csc_send_attr(uint32_t * c_wheel_revs, uint32_t * lwet, uint16_t * c_crank_revs, uint32_t * lcet, bool nfy_wheel, bool nfy_crank)
{
	/*
	 * In typical applications, the CSC Measurement characteristic is
	 * notified approximately once per second. This interval may vary
	 * and is determined by the Server and not required to be configurable
	 * by the Client.
	 */
	measurement_nfy(NULL, nfy_wheel ? *c_wheel_revs : 0, nfy_wheel ? (uint16_t)	*lwet : 0, nfy_crank ? *c_crank_revs : 0, nfy_crank ? (uint16_t)*lcet : 0);

}

/* ------> PRIVATE FUNCTIONS <------ */
// Callback when CCC (Client Characteristic Configuration) for CSC Measurement changes
static void csc_meas_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				     uint16_t value)
{
	csc_simulate = value == BT_GATT_CCC_NOTIFY;
}

// Callback when CCC for SC Control Point changes
static void ctrl_point_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				       uint16_t value)
{
	ctrl_point_configured = value == BT_GATT_CCC_INDICATE;
}

// Read callback for the Sensor Location characteristic
static ssize_t read_location(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr, void *buf,
			     uint16_t len, uint16_t offset)
{
	uint8_t *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 sizeof(*value));
}

// Read callback for the CSC Feature characteristic
static ssize_t read_csc_feature(struct bt_conn *conn,
				const struct bt_gatt_attr *attr, void *buf,
				uint16_t len, uint16_t offset)
{
	uint16_t csc_feature = CSC_FEATURE;

	return bt_gatt_attr_read(conn, attr, buf, len, offset,
				 &csc_feature, sizeof(csc_feature));
}

// Function to send indication for SC Control Point
static void ctrl_point_ind(struct bt_conn *conn, uint8_t req_op, uint8_t status,
			   const void *data, uint16_t data_len);


// Write callback for SC Control Point
static ssize_t write_ctrl_point(struct bt_conn *conn,
				const struct bt_gatt_attr *attr,
				const void *buf, uint16_t len, uint16_t offset,
				uint8_t flags)
{
	const struct write_sc_ctrl_point_req *req = buf;
	uint8_t status;
	int i;

	if (!ctrl_point_configured) {
		return BT_GATT_ERR(CSC_ERR_CCC_CONFIG);
	}

	if (!len) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	switch (req->op) {
	case SC_CP_OP_SET_CWR:
		if (len != sizeof(req->op) + sizeof(req->cwr)) {
			status = SC_CP_RSP_INVAL_PARAM;
			break;
		}

		c_wheel_revs = sys_le32_to_cpu(req->cwr);
		status = SC_CP_RSP_SUCCESS;
		break;
	case SC_CP_OP_UPDATE_LOC:
		if (len != sizeof(req->op) + sizeof(req->location)) {
			status = SC_CP_RSP_INVAL_PARAM;
			break;
		}

		/* Break if the requested location is the same as the current one */
		if (req->location == sensor_location) {
			status = SC_CP_RSP_SUCCESS;
			break;
		}

		/* Pre-set status */
		status = SC_CP_RSP_INVAL_PARAM;

		/* Check if the requested location is supported */
		for (i = 0; i < ARRAY_SIZE(supported_locations); i++) {
			if (supported_locations[i] == req->location) {
				sensor_location = req->location;
				status = SC_CP_RSP_SUCCESS;
				break;
			}
		}

		break;
	case SC_CP_OP_REQ_SUPP_LOC:
		if (len != sizeof(req->op)) {
			status = SC_CP_RSP_INVAL_PARAM;
			break;
		}

		/* Indicate supported locations and return */
		ctrl_point_ind(conn, req->op, SC_CP_RSP_SUCCESS,
			       &supported_locations,
			       sizeof(supported_locations));

		return len;
	default:
		status = SC_CP_RSP_OP_NOT_SUPP;
	}

	ctrl_point_ind(conn, req->op, status, NULL, 0);

	return len;
}

// Function to send indication for SC Control Point
static void ctrl_point_ind(struct bt_conn *conn, uint8_t req_op, uint8_t status,
			   const void *data, uint16_t data_len)
{
	struct sc_ctrl_point_ind *ind;
	uint8_t buf[sizeof(*ind) + data_len];

	ind = (void *) buf;
	ind->op = SC_CP_OP_RESPONSE;
	ind->req_op = req_op;
	ind->status = status;

	/* Send data (supported locations) if present */
	if (data && data_len) {
		memcpy(ind->data, data, data_len);
	}

	// Notify the SC Control Point characteristic
	bt_gatt_notify(conn, &csc_svc.attrs[8], buf, sizeof(buf));
}

// Function to send CSC Measurement notification
static void measurement_nfy(struct bt_conn *conn, uint32_t cwr, uint16_t lwet,
			    uint16_t ccr, uint16_t lcet)
{
	struct csc_measurement_nfy *nfy;
	uint8_t buf[sizeof(*nfy) +
		    (cwr ? sizeof(struct wheel_rev_data_nfy) : 0) +
		    (ccr ? sizeof(struct crank_rev_data_nfy) : 0)];
	uint16_t len = 0U;

	nfy = (void *) buf;
	nfy->flags = 0U;

	/* Send Wheel Revolution data if present */
	if (cwr) {
		struct wheel_rev_data_nfy data;

		nfy->flags |= CSC_WHEEL_REV_DATA_PRESENT;
		data.cwr = sys_cpu_to_le32(cwr);
		data.lwet = sys_cpu_to_le16(lwet);

		memcpy(nfy->data, &data, sizeof(data));
		len += sizeof(data);
	}

	/* Send Crank Revolution data if present */
	if (ccr) {
		struct crank_rev_data_nfy data;

		nfy->flags |= CSC_CRANK_REV_DATA_PRESENT;
		data.ccr = sys_cpu_to_le16(ccr);
		data.lcet = sys_cpu_to_le16(lcet);

		memcpy(nfy->data + len, &data, sizeof(data));
	}

	// Notify the CSC Measurement characteristic
	bt_gatt_notify(NULL, &csc_svc.attrs[1], buf, sizeof(buf));
}

// Callback when a connection is established
static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_INF("Connection failed (err 0x%02x)\n", err);
	} else {
		LOG_INF("Connected\n");
	}
}

// Callback when a connection is disconnected
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected (reason 0x%02x)\n", reason);
}

/* ------> INTERRUPT HANDLERS <------ */


