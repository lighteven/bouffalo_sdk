/** @file
 *  @brief Handsfree Profile handling.
 */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_INCLUDE_BLUETOOTH_HFP_HF_H_
#define ZEPHYR_INCLUDE_BLUETOOTH_HFP_HF_H_

/**
 * @brief Hands Free Profile (HFP)
 * @defgroup bt_hfp Hands Free Profile (HFP)
 * @ingroup bluetooth
 * @{
 */

#include <bluetooth.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AT Commands */
enum bt_hfp_hf_at_cmd {
	BT_HFP_HF_ATA,
	BT_HFP_HF_AT_CHUP,
	BT_HFP_HF_AT_VGM,
	BT_HFP_HF_AT_VGS,
	BT_HFP_HF_AT_DDD,
	BT_HFP_HF_AT_NREC,
	BT_HFP_HF_AT_BVRA,
	BT_HFP_HF_AT_BINP,
	BT_HFP_ACCEPT_INCOMING_CALLER_ID,
	BT_HFP_SET_MIC_VOL,
	BT_HFP_QUERY_LIST_CALLS,
	BT_HFP_RESPONSE_CALLS,
	BT_HFP_SUBSCRIBE_NUM_INFO,
	BT_HFP_SEND_INDICATOR,
	BT_HFP_UPDATE_INDICATOR,
};

/*
 * Command complete types for the application
 */
#define HFP_HF_CMD_OK             0
#define HFP_HF_CMD_ERROR          1
#define HFP_HF_CMD_CME_ERROR      2
#define HFP_HF_CMD_UNKNOWN_ERROR  4

/** @brief HFP HF Command completion field */
struct bt_hfp_hf_cmd_complete {
	/* Command complete status */
	uint8_t type;
	/* CME error number to be added */
	uint8_t cme;
};

/** @brief HFP profile application callback */
struct bt_hfp_hf_cb {
	/** HF connected callback to application
	 *
	 *  If this callback is provided it will be called whenever the
	 *  connection completes.
	 *
	 *  @param conn Connection object.
	 */
	void (*connected)(struct bt_conn *conn);
	/** HF disconnected callback to application
	 *
	 *  If this callback is provided it will be called whenever the
	 *  connection gets disconnected, including when a connection gets
	 *  rejected or cancelled or any error in SLC establisment.
	 *
	 *  @param conn Connection object.
	 */
	void (*disconnected)(struct bt_conn *conn);
	/** HF indicator Callback
	 *
	 *  This callback provides service indicator value to the application
	 *
	 *  @param conn Connection object.
	 *  @param value service indicator value received from the AG.
	 */
	void (*service)(struct bt_conn *conn, uint32_t value);
	/** HF indicator Callback
	 *
	 *  This callback provides call indicator value to the application
	 *
	 *  @param conn Connection object.
	 *  @param value call indicator value received from the AG.
	 */
	void (*call)(struct bt_conn *conn, uint32_t value);
	/** HF indicator Callback
	 *
	 *  This callback provides call setup indicator value to the application
	 *
	 *  @param conn Connection object.
	 *  @param value call setup indicator value received from the AG.
	 */
	void (*call_setup)(struct bt_conn *conn, uint32_t value);
	/** HF indicator Callback
	 *
	 *  This callback provides call held indicator value to the application
	 *
	 *  @param conn Connection object.
	 *  @param value call held indicator value received from the AG.
	 */
	void (*call_held)(struct bt_conn *conn, uint32_t value);
	/** HF indicator Callback
	 *
	 *  This callback provides signal indicator value to the application
	 *
	 *  @param conn Connection object.
	 *  @param value signal indicator value received from the AG.
	 */
	void (*signal)(struct bt_conn *conn, uint32_t value);
	/** HF indicator Callback
	 *
	 *  This callback provides roaming indicator value to the application
	 *
	 *  @param conn Connection object.
	 *  @param value roaming indicator value received from the AG.
	 */
	void (*roam)(struct bt_conn *conn, uint32_t value);
	/** HF indicator Callback
	 *
	 *  This callback battery service indicator value to the application
	 *
	 *  @param conn Connection object.
	 *  @param value battery indicator value received from the AG.
	 */
	void (*battery)(struct bt_conn *conn, uint32_t value);
	/** VGS indicator Callback
	 *
	 *  This callback vgs indicator value to the application
	 *
	 *  @param conn Connection object.
	 *  @param value VGS.
	 */
	void (*vgs_indication)(struct bt_conn *conn, uint32_t value);
		/** VGM indicator Callback
	 *
	 *  This callback vgs indicator value to the application
	 *
	 *  @param conn Connection object.
	 *  @param value VGS.
	 */
	void (*vgm_indication)(struct bt_conn *conn, uint32_t value);


	/** HF incoming call Ring indication callback to application
	 *
	 *  If this callback is provided it will be called whenever there
	 *  is an incoming call.
	 *
	 *  @param conn Connection object.
	 */
	void (*ring_indication)(struct bt_conn *conn);
	/** HF notify command completed callback to application
	 *
	 *  The command sent from the application is notified about its status
	 *
	 *  @param conn Connection object.
	 *  @param cmd structure contains status of the command including cme.
	 */
	void (*cmd_complete_cb)(struct bt_conn *conn,
			      struct bt_hfp_hf_cmd_complete *cmd);
};

/** @brief Register HFP HF profile
 *
 *  Make hfp initiate hfp connection.
 *
 *  @param conn bredr connection.
 *
 *  @return 0 in case of success or negative value in case of error.
 */
int bt_hfp_hf_initiate_connect(struct bt_conn *conn);

/** @brief Register HFP HF profile
 *
 *  Register Handsfree profile callbacks to monitor the state and get the
 *  required HFP details to display.
 *
 *  @param cb callback structure.
 *
 *  @return 0 in case of success or negative value in case of error.
 */
int bt_hfp_hf_register(struct bt_hfp_hf_cb *cb);
/**
* @brief Initialize HFP_HF layer
*/
int bt_hfp_hf_init(void);

/** @brief Handsfree client Send AT
 *
 *  Send specific AT commands to handsfree client profile.
 *
 *  @param conn Connection object.
 *  @param cmd AT command to be sent.
 *
 *  @return 0 in case of success or negative value in case of error.
 */
int bt_hfp_hf_send_cmd(struct bt_conn *conn, enum bt_hfp_hf_at_cmd cmd, const char *format);

int bt_hfp_hf_send_cmd_arg(struct bt_conn *conn, enum bt_hfp_hf_at_cmd cmd, int arg1);

int bt_hfp_hf_send_disconnect(struct bt_conn *conn);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_BLUETOOTH_HFP_HF_H_ */
