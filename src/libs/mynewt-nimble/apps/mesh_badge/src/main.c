/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "console/console.h"
#include "host/ble_gap.h"
#include "mesh/glue.h"
#include "services/gap/ble_svc_gap.h"
#include "base64/base64.h"

#include "mesh_badge.h"
#include "mesh.h"
#include "board.h"

static char badge_name[MYNEWT_VAL(BLE_SVC_GAP_DEVICE_NAME_MAX_LENGTH)];

#define MESH_BADGE_NAME_ENCODE_SIZE      \
    BASE64_ENCODE_SIZE(sizeof(badge_name))

static bool reset_mesh;

void print_addr(const void *addr)
{
	const uint8_t *u8p;

	u8p = addr;
	MODLOG_DFLT(INFO, "%02x:%02x:%02x:%02x:%02x:%02x",
		    u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}

static void
print_conn_desc(struct ble_gap_conn_desc *desc)
{
	MODLOG_DFLT(INFO, "handle=%d our_ota_addr_type=%d our_ota_addr=",
		    desc->conn_handle, desc->our_ota_addr.type);
	print_addr(desc->our_ota_addr.val);
	MODLOG_DFLT(INFO, " our_id_addr_type=%d our_id_addr=",
		    desc->our_id_addr.type);
	print_addr(desc->our_id_addr.val);
	MODLOG_DFLT(INFO, " peer_ota_addr_type=%d peer_ota_addr=",
		    desc->peer_ota_addr.type);
	print_addr(desc->peer_ota_addr.val);
	MODLOG_DFLT(INFO, " peer_id_addr_type=%d peer_id_addr=",
		    desc->peer_id_addr.type);
	print_addr(desc->peer_id_addr.val);
	MODLOG_DFLT(INFO, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
			  "encrypted=%d authenticated=%d bonded=%d\n",
		    desc->conn_itvl, desc->conn_latency,
		    desc->supervision_timeout,
		    desc->sec_state.encrypted,
		    desc->sec_state.authenticated,
		    desc->sec_state.bonded);
}

static int gap_event(struct ble_gap_event *event, void *arg);

static void advertise(void)
{
	uint8_t own_addr_type;
	struct ble_gap_adv_params adv_params;
	struct ble_hs_adv_fields fields;
	const char *name;
	int rc;

	/* Figure out address to use while advertising (no privacy for now) */
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	if (rc != 0) {
		MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
		return;
	}

	/**
	 *  Set the advertisement data included in our advertisements:
	 *     o Flags (indicates advertisement type and other general info).
	 *     o Advertising tx power.
	 *     o Device name.
	 *     o 16-bit service UUIDs (alert notifications).
	 */

	memset(&fields, 0, sizeof fields);

	/* Advertise two flags:
	 *     o Discoverability in forthcoming advertisement (general)
	 *     o BLE-only (BR/EDR unsupported).
	 */
	fields.flags = BLE_HS_ADV_F_DISC_GEN |
		       BLE_HS_ADV_F_BREDR_UNSUP;

#if 0
	/* Indicate that the TX power level field should be included; have the
	 * stack fill this value automatically.  This is done by assiging the
	 * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
	 */
	fields.tx_pwr_lvl_is_present = 1;
	fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
#endif

	name = ble_svc_gap_device_name();
	fields.name = (uint8_t *)name;
	fields.name_len = (uint8_t) strlen(name);
	fields.name_is_complete = 1;

	rc = ble_gap_adv_set_fields(&fields);
	if (rc != 0) {
		MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
		return;
	}

	/* Begin advertising. */
	memset(&adv_params, 0, sizeof adv_params);
	adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
	adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
	rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
			       &adv_params, gap_event, NULL);
	if (rc != 0) {
		MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
		return;
	}
}

static void passkey_display(uint16_t conn_handle)
{
	char buf[20];
	struct ble_sm_io pk;
	int rc;

	bt_rand(&pk.passkey, sizeof(pk.passkey));
	/* Max value is 999999 */
	pk.passkey %= 1000000;
	pk.action = BLE_SM_IOACT_DISP;

	rc = ble_sm_inject_io(conn_handle, &pk);
	assert(rc == 0);

	snprintk(buf, sizeof(buf), "Passkey:\n%06lu", pk.passkey);

	printk("%s\n", buf);
	board_show_text(buf, false, K_FOREVER);
}

static void pairing_complete(uint16_t conn_handle, bool bonded)
{
	printk("Pairing Complete\n");
	board_show_text("Pairing Complete", false, K_SECONDS(2));
}

static void pairing_failed(uint16_t conn_handle)
{
	printk("Pairing Failed\n");
	board_show_text("Pairing Failed", false, K_SECONDS(2));
}

static void connected(uint16_t conn_handle, int err)
{
	printk("Connected (err 0x%02x)\n", err);

	if (err) {
		board_show_text("Connection failed", false, K_SECONDS(2));
	} else {
		board_show_text("Connected", false, K_FOREVER);
	}
}

static void disconnected(uint16_t conn_handle, int reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);

	if (strcmp(MYNEWT_VAL(BLE_SVC_GAP_DEVICE_NAME), bt_get_name()) != 0 &&
	    !mesh_is_initialized()) {
		/* Mesh will take over advertising control */
		ble_gap_adv_stop();
		mesh_start();
	} else {
		board_show_text("Disconnected", false, K_SECONDS(2));
	}
}

static int gap_event(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc desc;
	int rc;

	switch (event->type) {
		case BLE_GAP_EVENT_CONNECT:
			/* A new connection was established or a connection attempt failed. */
			MODLOG_DFLT(INFO, "connection %s; status=%d ",
				    event->connect.status == 0 ? "established" : "failed",
				    event->connect.status);
			if (event->connect.status == 0) {
				rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
				assert(rc == 0);
				print_conn_desc(&desc);
				connected(event->connect.conn_handle,
					  event->connect.status);
			}
			MODLOG_DFLT(INFO, "\n");

			if (event->connect.status != 0) {
				/* Connection failed; resume advertising. */
				advertise();
			}
			return 0;

		case BLE_GAP_EVENT_DISCONNECT:
			MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
			print_conn_desc(&event->disconnect.conn);
			MODLOG_DFLT(INFO, "\n");

			/* Connection terminated; resume advertising. */
			advertise();

			disconnected(event->disconnect.conn.conn_handle,
				     event->disconnect.reason);
			return 0;

		case BLE_GAP_EVENT_CONN_UPDATE:
			/* The central has updated the connection parameters. */
			MODLOG_DFLT(INFO, "connection updated; status=%d ",
				    event->conn_update.status);
			rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
			assert(rc == 0);
			print_conn_desc(&desc);
			MODLOG_DFLT(INFO, "\n");
			return 0;

		case BLE_GAP_EVENT_ENC_CHANGE:
			/* Encryption has been enabled or disabled for this connection. */
			MODLOG_DFLT(INFO, "encryption change event; status=%d ",
				    event->enc_change.status);
			rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
			assert(rc == 0);
			print_conn_desc(&desc);
			MODLOG_DFLT(INFO, "\n");

			if (desc.sec_state.bonded) {
				pairing_complete(event->enc_change.conn_handle, true);
			} else if(desc.sec_state.encrypted) {
				pairing_complete(event->enc_change.conn_handle, false);
			} else {
				pairing_failed(event->enc_change.conn_handle);
			}
			return 0;

		case BLE_GAP_EVENT_PASSKEY_ACTION:
			MODLOG_DFLT(INFO, "passkey action event; conn_handle=%d action=%d numcmp=%d\n",
				    event->passkey.conn_handle,
				    event->passkey.params.action,
				    event->passkey.params.numcmp);
			passkey_display(event->passkey.conn_handle);
			return 0;

		case BLE_GAP_EVENT_REPEAT_PAIRING:
			/* We already have a bond with the peer, but it is attempting to
			 * establish a new secure link.  This app sacrifices security for
			 * convenience: just throw away the old bond and accept the new link.
			 */

			/* Delete the old bond. */
			rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
			assert(rc == 0);
			ble_store_util_delete_peer(&desc.peer_id_addr);

			/* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
			 * continue with the pairing operation.
			 */
			return BLE_GAP_REPEAT_PAIRING_RETRY;

	}

	return 0;
}

static void on_sync(void)
{
	int err;
	ble_addr_t addr;

	/* Use NRPA */
	err = ble_hs_id_gen_rnd(1, &addr);
	assert(err == 0);
	err = ble_hs_id_set_rnd(addr.val);
	assert(err == 0);

	printk("Bluetooth initialized\n");

	err = mesh_init(addr.type);
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	printk("Mesh initialized\n");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	if (reset_mesh) {
		bt_mesh_reset();
		reset_mesh = false;
	}

	if (!mesh_is_initialized()) {
		advertise();
	} else {
		printk("Already provisioned\n");
		ble_svc_gap_device_name_set(bt_get_name());
	}

	board_refresh_display();

	printk("Board started\n");
}

void schedule_mesh_reset(void)
{
	reset_mesh = true;
}

static void on_reset(int reason)
{
	MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

const char *bt_get_name(void)
{
	char buf[MESH_BADGE_NAME_ENCODE_SIZE];
	int rc, len;

	rc = conf_get_stored_value("mesh_badge/badge_name",
				   buf, sizeof(buf));
	if (rc == OS_ENOENT) {
		bt_set_name(MYNEWT_VAL(BLE_SVC_GAP_DEVICE_NAME));
	} else {
		assert(rc == 0);
	}

	memset(badge_name, '\0', sizeof(badge_name));
	len = base64_decode(buf, badge_name);
	if (len < 0) {
		bt_set_name(MYNEWT_VAL(BLE_SVC_GAP_DEVICE_NAME));
	}

	return badge_name;
}

int bt_set_name(const char *name)
{
	char buf[MESH_BADGE_NAME_ENCODE_SIZE];
	int rc;

	memset(badge_name, '\0', sizeof(badge_name));
	memcpy(badge_name, name, strlen(name));
	base64_encode(badge_name, sizeof(badge_name), buf, 1);
	rc = conf_save_one("mesh_badge/badge_name", buf);
	assert(rc == 0);

	return 0;
}

int main(void)
{
	int err;

	/* Initialize OS */
	sysinit();

	err = board_init();
	if (err) {
		printk("board init failed (err %d)\n", err);
		assert(err == 0);
	}

	/* Initialize the NimBLE host configuration. */
	ble_hs_cfg.reset_cb = on_reset;
	ble_hs_cfg.sync_cb = on_sync;
	ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
	ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
	ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_DISP_ONLY;

	err = gatt_svr_init();
	assert(err == 0);

	/*
	 * As the last thing, process events from default event queue.
	 */
	while (1) {
		os_eventq_run(os_eventq_dflt_get());
	}
	return 0;
}
