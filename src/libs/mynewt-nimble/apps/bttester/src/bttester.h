/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/* bttester.h - Bluetooth tester headers */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BTTESTER_H__
#define __BTTESTER_H__

#include "syscfg/syscfg.h"
#include "host/ble_gatt.h"

#if MYNEWT_VAL(BLE_MESH)
#include "mesh/glue.h"
#else
#include "glue.h"
#endif

#define BTP_MTU MYNEWT_VAL(BTTESTER_BTP_DATA_SIZE_MAX)
#define BTP_DATA_MAX_SIZE (BTP_MTU - sizeof(struct btp_hdr))

#define BTP_INDEX_NONE		0xff

#define BTP_SERVICE_ID_CORE	0
#define BTP_SERVICE_ID_GAP	1
#define BTP_SERVICE_ID_GATT	2
#define BTP_SERVICE_ID_L2CAP	3
#define BTP_SERVICE_ID_MESH	4

#define BTP_STATUS_SUCCESS	0x00
#define BTP_STATUS_FAILED	0x01
#define BTP_STATUS_UNKNOWN_CMD	0x02
#define BTP_STATUS_NOT_READY	0x03

#define SYS_LOG_DBG(fmt, ...) \
	if (MYNEWT_VAL(BTTESTER_DEBUG)) { \
		console_printf("[DBG] %s: " fmt "\n", \
			       __func__, ## __VA_ARGS__); \
	}
#define SYS_LOG_INF(fmt, ...)   console_printf("[INF] %s: " fmt "\n", \
					       __func__, ## __VA_ARGS__);
#define SYS_LOG_ERR(fmt, ...)   console_printf("[WRN] %s: " fmt "\n", \
					       __func__, ## __VA_ARGS__);

#define SYS_LOG_LEVEL SYS_LOG_LEVEL_DEBUG
#define SYS_LOG_DOMAIN "bttester"

#define sys_cpu_to_le32 htole32
#define sys_le32_to_cpu le32toh
#define sys_cpu_to_le16 htole16

struct btp_hdr {
	u8_t  service;
	u8_t  opcode;
	u8_t  index;
	u16_t len;
	u8_t  data[0];
} __packed;

#define BTP_STATUS			0x00
struct btp_status {
	u8_t code;
} __packed;

/* Core Service */
#define CORE_READ_SUPPORTED_COMMANDS	0x01
struct core_read_supported_commands_rp {
	u8_t data[0];
} __packed;

#define CORE_READ_SUPPORTED_SERVICES	0x02
struct core_read_supported_services_rp {
	u8_t data[0];
} __packed;

#define CORE_REGISTER_SERVICE		0x03
struct core_register_service_cmd {
	u8_t id;
} __packed;

#define CORE_UNREGISTER_SERVICE		0x04
struct core_unregister_service_cmd {
	u8_t id;
} __packed;

/* events */
#define CORE_EV_IUT_READY		0x80

/* GAP Service */
/* commands */
#define GAP_READ_SUPPORTED_COMMANDS	0x01
struct gap_read_supported_commands_rp {
	u8_t data[0];
} __packed;

#define GAP_READ_CONTROLLER_INDEX_LIST	0x02
struct gap_read_controller_index_list_rp {
	u8_t num;
	u8_t index[0];
} __packed;

#define GAP_SETTINGS_POWERED		0
#define GAP_SETTINGS_CONNECTABLE	1
#define GAP_SETTINGS_FAST_CONNECTABLE	2
#define GAP_SETTINGS_DISCOVERABLE	3
#define GAP_SETTINGS_BONDABLE		4
#define GAP_SETTINGS_LINK_SEC_3		5
#define GAP_SETTINGS_SSP		6
#define GAP_SETTINGS_BREDR		7
#define GAP_SETTINGS_HS			8
#define GAP_SETTINGS_LE			9
#define GAP_SETTINGS_ADVERTISING	10
#define GAP_SETTINGS_SC			11
#define GAP_SETTINGS_DEBUG_KEYS		12
#define GAP_SETTINGS_PRIVACY		13
#define GAP_SETTINGS_CONTROLLER_CONFIG	14
#define GAP_SETTINGS_STATIC_ADDRESS	15

#define GAP_READ_CONTROLLER_INFO	0x03
struct gap_read_controller_info_rp {
	u8_t  address[6];
	u32_t supported_settings;
	u32_t current_settings;
	u8_t  cod[3];
	u8_t  name[249];
	u8_t  short_name[11];
} __packed;

#define GAP_RESET			0x04
struct gap_reset_rp {
	u32_t current_settings;
} __packed;

#define GAP_SET_POWERED			0x05
struct gap_set_powered_cmd {
	u8_t powered;
} __packed;
struct gap_set_powered_rp {
	u32_t current_settings;
} __packed;

#define GAP_SET_CONNECTABLE		0x06
struct gap_set_connectable_cmd {
	u8_t connectable;
} __packed;
struct gap_set_connectable_rp {
	u32_t current_settings;
} __packed;

#define GAP_SET_FAST_CONNECTABLE	0x07
struct gap_set_fast_connectable_cmd {
	u8_t fast_connectable;
} __packed;
struct gap_set_fast_connectable_rp {
	u32_t current_settings;
} __packed;

#define GAP_NON_DISCOVERABLE		0x00
#define GAP_GENERAL_DISCOVERABLE	0x01
#define GAP_LIMITED_DISCOVERABLE	0x02

#define GAP_SET_DISCOVERABLE		0x08
struct gap_set_discoverable_cmd {
	u8_t discoverable;
} __packed;
struct gap_set_discoverable_rp {
	u32_t current_settings;
} __packed;

#define GAP_SET_BONDABLE		0x09
struct gap_set_bondable_cmd {
	u8_t bondable;
} __packed;
struct gap_set_bondable_rp {
	u32_t current_settings;
} __packed;

#define GAP_START_ADVERTISING	0x0a
struct gap_start_advertising_cmd {
	u8_t adv_data_len;
	u8_t scan_rsp_len;
	u8_t adv_data[0];
	u8_t scan_rsp[0];
} __packed;
struct gap_start_advertising_rp {
	u32_t current_settings;
} __packed;

#define GAP_STOP_ADVERTISING		0x0b
struct gap_stop_advertising_rp {
	u32_t current_settings;
} __packed;

#define GAP_DISCOVERY_FLAG_LE			0x01
#define GAP_DISCOVERY_FLAG_BREDR		0x02
#define GAP_DISCOVERY_FLAG_LIMITED		0x04
#define GAP_DISCOVERY_FLAG_LE_ACTIVE_SCAN	0x08
#define GAP_DISCOVERY_FLAG_LE_OBSERVE		0x10

#define GAP_START_DISCOVERY		0x0c
struct gap_start_discovery_cmd {
	u8_t flags;
} __packed;

#define GAP_STOP_DISCOVERY		0x0d

#define GAP_CONNECT			0x0e
struct gap_connect_cmd {
	u8_t address_type;
	u8_t address[6];
} __packed;

#define GAP_DISCONNECT			0x0f
struct gap_disconnect_cmd {
	u8_t  address_type;
	u8_t  address[6];
} __packed;

#define GAP_IO_CAP_DISPLAY_ONLY		0
#define GAP_IO_CAP_DISPLAY_YESNO	1
#define GAP_IO_CAP_KEYBOARD_ONLY	2
#define GAP_IO_CAP_NO_INPUT_OUTPUT	3
#define GAP_IO_CAP_KEYBOARD_DISPLAY	4

#define GAP_SET_IO_CAP			0x10
struct gap_set_io_cap_cmd {
	u8_t io_cap;
} __packed;

#define GAP_PAIR			0x11
struct gap_pair_cmd {
	u8_t address_type;
	u8_t address[6];
} __packed;

#define GAP_UNPAIR			0x12
struct gap_unpair_cmd {
	u8_t address_type;
	u8_t address[6];
} __packed;

#define GAP_PASSKEY_ENTRY		0x13
struct gap_passkey_entry_cmd {
	u8_t  address_type;
	u8_t  address[6];
	u32_t passkey;
} __packed;

#define GAP_PASSKEY_CONFIRM		0x14
struct gap_passkey_confirm_cmd {
	u8_t address_type;
	u8_t address[6];
	u8_t match;
} __packed;

#define GAP_START_DIRECT_ADV		0x15
struct gap_start_direct_adv_cmd {
	u8_t address_type;
	u8_t address[6];
	u8_t high_duty;
} __packed;

#define GAP_CONN_PARAM_UPDATE		0x16
struct gap_conn_param_update_cmd {
    u8_t address_type;
    u8_t address[6];
    u16_t conn_itvl_min;
    u16_t conn_itvl_max;
    u16_t conn_latency;
    u16_t supervision_timeout;
} __packed;

#define GAP_PAIRING_CONSENT_RSP		0x17
struct gap_pairing_consent_rsp_cmd {
    u8_t address_type;
    u8_t address[6];
    u8_t consent;
} __packed;

#define GAP_OOB_LEGACY_SET_DATA		0x18
struct gap_oob_legacy_set_data_cmd {
    u8_t oob_data[16];
} __packed;

#define GAP_OOB_SC_GET_LOCAL_DATA		0x19
struct gap_oob_sc_get_local_data_rp {
    u8_t r[16];
    u8_t c[16];
} __packed;

#define GAP_OOB_SC_SET_REMOTE_DATA		0x1a
struct gap_oob_sc_set_remote_data_cmd {
    u8_t r[16];
    u8_t c[16];
} __packed;

#define GAP_SET_MITM		0x1b
struct gap_set_mitm_cmd {
    u8_t mitm;
} __packed;

/* events */
#define GAP_EV_NEW_SETTINGS		0x80
struct gap_new_settings_ev {
	u32_t current_settings;
} __packed;

#define GAP_DEVICE_FOUND_FLAG_RSSI	0x01
#define GAP_DEVICE_FOUND_FLAG_AD	0x02
#define GAP_DEVICE_FOUND_FLAG_SD	0x04

#define GAP_EV_DEVICE_FOUND		0x81
struct gap_device_found_ev {
	u8_t  address_type;
	u8_t  address[6];
	s8_t   rssi;
	u8_t  flags;
	u16_t eir_data_len;
	u8_t  eir_data[0];
} __packed;

#define GAP_EV_DEVICE_CONNECTED		0x82
struct gap_device_connected_ev {
	u8_t address_type;
	u8_t address[6];
	u16_t conn_itvl;
	u16_t conn_latency;
	u16_t supervision_timeout;
} __packed;

#define GAP_EV_DEVICE_DISCONNECTED	0x83
struct gap_device_disconnected_ev {
	u8_t address_type;
	u8_t address[6];
} __packed;

#define GAP_EV_PASSKEY_DISPLAY		0x84
struct gap_passkey_display_ev {
	u8_t  address_type;
	u8_t  address[6];
	u32_t passkey;
} __packed;

#define GAP_EV_PASSKEY_ENTRY_REQ	0x85
struct gap_passkey_entry_req_ev {
	u8_t address_type;
	u8_t address[6];
} __packed;

#define GAP_EV_PASSKEY_CONFIRM_REQ	0x86
struct gap_passkey_confirm_req_ev {
	u8_t  address_type;
	u8_t  address[6];
	u32_t passkey;
} __packed;

#define GAP_EV_IDENTITY_RESOLVED	0x87
struct gap_identity_resolved_ev {
	u8_t address_type;
	u8_t address[6];
	u8_t identity_address_type;
	u8_t identity_address[6];
} __packed;

#define GAP_EV_CONN_PARAM_UPDATE	0x88
struct gap_conn_param_update_ev {
    u8_t address_type;
    u8_t address[6];
    u16_t conn_itvl;
    u16_t conn_latency;
    u16_t supervision_timeout;
} __packed;

#define GAP_EV_SEC_LEVEL_CHANGED	0x89
struct gap_sec_level_changed_ev {
    u8_t address_type;
    u8_t address[6];
    u8_t level;
} __packed;

#define GAP_EV_PAIRING_CONSENT_REQ	0x8a
struct gap_pairing_consent_req_ev {
    u8_t address_type;
    u8_t address[6];
} __packed;

/* GATT Service */
/* commands */
#define GATT_READ_SUPPORTED_COMMANDS	0x01
struct gatt_read_supported_commands_rp {
	u8_t data[0];
} __packed;

#define GATT_SERVICE_PRIMARY		0x00
#define GATT_SERVICE_SECONDARY		0x01

#define GATT_ADD_SERVICE		0x02
struct gatt_add_service_cmd {
	u8_t type;
	u8_t uuid_length;
	u8_t uuid[0];
} __packed;
struct gatt_add_service_rp {
	u16_t svc_id;
} __packed;

#define GATT_ADD_CHARACTERISTIC		0x03
struct gatt_add_characteristic_cmd {
	u16_t svc_id;
	u8_t properties;
	u8_t permissions;
	u8_t uuid_length;
	u8_t uuid[0];
} __packed;
struct gatt_add_characteristic_rp {
	u16_t char_id;
} __packed;

#define GATT_ADD_DESCRIPTOR		0x04
struct gatt_add_descriptor_cmd {
	u16_t char_id;
	u8_t permissions;
	u8_t uuid_length;
	u8_t uuid[0];
} __packed;
struct gatt_add_descriptor_rp {
	u16_t desc_id;
} __packed;

#define GATT_ADD_INCLUDED_SERVICE	0x05
struct gatt_add_included_service_cmd {
	u16_t svc_id;
} __packed;
struct gatt_add_included_service_rp {
	u16_t included_service_id;
} __packed;

#define GATT_SET_VALUE			0x06
	struct gatt_set_value_cmd {
	u16_t attr_id;
	u16_t len;
	u8_t value[0];
} __packed;

#define GATT_START_SERVER		0x07
struct gatt_start_server_rp {
	u16_t db_attr_off;
	u8_t db_attr_cnt;
} __packed;

#define GATT_SET_ENC_KEY_SIZE		0x09
struct gatt_set_enc_key_size_cmd {
	u16_t attr_id;
	u8_t key_size;
} __packed;

/* Gatt Client */
struct gatt_service {
	u16_t start_handle;
	u16_t end_handle;
	u8_t uuid_length;
	u8_t uuid[0];
} __packed;

struct gatt_included {
	u16_t included_handle;
	struct gatt_service service;
} __packed;

struct gatt_characteristic {
	u16_t characteristic_handle;
	u16_t value_handle;
	u8_t properties;
	u8_t uuid_length;
	u8_t uuid[0];
} __packed;

struct gatt_descriptor {
	u16_t descriptor_handle;
	u8_t uuid_length;
	u8_t uuid[0];
} __packed;

#define GATT_EXCHANGE_MTU		0x0a

#define GATT_DISC_ALL_PRIM_SVCS		0x0b
struct gatt_disc_all_prim_svcs_cmd {
    u8_t address_type;
    u8_t address[6];
} __packed;
struct gatt_disc_all_prim_svcs_rp {
    u8_t services_count;
    struct gatt_service services[0];
} __packed;

#define GATT_DISC_PRIM_UUID		0x0c
struct gatt_disc_prim_uuid_cmd {
	u8_t address_type;
	u8_t address[6];
	u8_t uuid_length;
	u8_t uuid[0];
} __packed;
struct gatt_disc_prim_uuid_rp {
	u8_t services_count;
	struct gatt_service services[0];
} __packed;

#define GATT_FIND_INCLUDED		0x0d
struct gatt_find_included_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t start_handle;
	u16_t end_handle;
} __packed;
struct gatt_find_included_rp {
	u8_t services_count;
	struct gatt_included included[0];
} __packed;

#define GATT_DISC_ALL_CHRC		0x0e
struct gatt_disc_all_chrc_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t start_handle;
	u16_t end_handle;
} __packed;
struct gatt_disc_chrc_rp {
	u8_t characteristics_count;
	struct gatt_characteristic characteristics[0];
} __packed;

#define GATT_DISC_CHRC_UUID		0x0f
struct gatt_disc_chrc_uuid_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t start_handle;
	u16_t end_handle;
	u8_t uuid_length;
	u8_t uuid[0];
} __packed;

#define GATT_DISC_ALL_DESC		0x10
struct gatt_disc_all_desc_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t start_handle;
	u16_t end_handle;
} __packed;
struct gatt_disc_all_desc_rp {
	u8_t descriptors_count;
	struct gatt_descriptor descriptors[0];
} __packed;

#define GATT_READ			0x11
struct gatt_read_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t handle;
} __packed;
struct gatt_read_rp {
	u8_t att_response;
	u16_t data_length;
	u8_t data[0];
} __packed;

#define GATT_READ_UUID			0x12
struct gatt_read_uuid_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t start_handle;
	u16_t end_handle;
	u8_t uuid_length;
	u8_t uuid[0];
} __packed;

#define GATT_READ_LONG			0x13
struct gatt_read_long_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t handle;
	u16_t offset;
} __packed;

#define GATT_READ_MULTIPLE		0x14
struct gatt_read_multiple_cmd {
	u8_t address_type;
	u8_t address[6];
	u8_t handles_count;
	u16_t handles[0];
} __packed;

#define GATT_WRITE_WITHOUT_RSP		0x15
struct gatt_write_without_rsp_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t handle;
	u16_t data_length;
	u8_t data[0];
} __packed;

#define GATT_SIGNED_WRITE_WITHOUT_RSP	0x16
struct gatt_signed_write_without_rsp_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t handle;
	u16_t data_length;
	u8_t data[0];
} __packed;

#define GATT_WRITE			0x17
struct gatt_write_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t handle;
	u16_t data_length;
	u8_t data[0];
} __packed;

#define GATT_WRITE_LONG			0x18
struct gatt_write_long_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t handle;
	u16_t offset;
	u16_t data_length;
	u8_t data[0];
} __packed;

#define GATT_RELIABLE_WRITE		0x19
struct gatt_reliable_write_cmd {
    u8_t address_type;
    u8_t address[6];
    u16_t handle;
    u16_t offset;
    u16_t data_length;
    u8_t data[0];
} __packed;

#define GATT_CFG_NOTIFY			0x1a
#define GATT_CFG_INDICATE		0x1b
struct gatt_cfg_notify_cmd {
	u8_t address_type;
	u8_t address[6];
	u8_t enable;
	u16_t ccc_handle;
} __packed;

#define GATT_GET_ATTRIBUTES		0x1c
struct gatt_get_attributes_cmd {
	u16_t start_handle;
	u16_t end_handle;
	u8_t type_length;
	u8_t type[0];
} __packed;
struct gatt_get_attributes_rp {
	u8_t attrs_count;
	u8_t attrs[0];
} __packed;
struct gatt_attr {
	u16_t handle;
	u8_t permission;
	u8_t type_length;
	u8_t type[0];
} __packed;

#define GATT_GET_ATTRIBUTE_VALUE	0x1d
struct gatt_get_attribute_value_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t handle;
} __packed;
struct gatt_get_attribute_value_rp {
	u8_t att_response;
	u16_t value_length;
	u8_t value[0];
} __packed;

#define GATT_CHANGE_DATABASE		0x1e
struct gatt_change_database {
    u16_t start_handle;
    u16_t end_handle;
    u8_t visibility;
} __packed;

/* GATT events */
#define GATT_EV_NOTIFICATION		0x80
struct gatt_notification_ev {
	u8_t address_type;
	u8_t address[6];
	u8_t type;
	u16_t handle;
	u16_t data_length;
	u8_t data[0];
} __packed;

#define GATT_EV_ATTR_VALUE_CHANGED	0x81
struct gatt_attr_value_changed_ev {
	u16_t handle;
	u16_t data_length;
	u8_t data[0];
} __packed;

static inline void tester_set_bit(u8_t *addr, unsigned int bit)
{
	u8_t *p = addr + (bit / 8);

	*p |= BIT(bit % 8);
}

static inline u8_t tester_test_bit(const u8_t *addr, unsigned int bit)
{
	const u8_t *p = addr + (bit / 8);

	return *p & BIT(bit % 8);
}

/* L2CAP Service */
/* commands */
#define L2CAP_READ_SUPPORTED_COMMANDS	0x01
struct l2cap_read_supported_commands_rp {
	u8_t data[0];
} __packed;

#define L2CAP_CONNECT			0x02
struct l2cap_connect_cmd {
	u8_t address_type;
	u8_t address[6];
	u16_t psm;
} __packed;

struct l2cap_connect_rp {
	u8_t chan_id;
} __packed;

#define L2CAP_DISCONNECT		0x03
struct l2cap_disconnect_cmd {
	u8_t chan_id;
} __packed;

#define L2CAP_SEND_DATA			0x04
struct l2cap_send_data_cmd {
	u8_t chan_id;
	u16_t data_len;
	u8_t data[];
} __packed;

#define L2CAP_TRANSPORT_BREDR		0x00
#define L2CAP_TRANSPORT_LE		0x01

#define L2CAP_LISTEN			0x05
struct l2cap_listen_cmd {
	u16_t psm;
	u8_t transport;
} __packed;

#define L2CAP_ACCEPT_CONNECTION		0x06
struct l2cap_accept_connection_cmd {
	u8_t chan_id;
	u16_t result;
} __packed;

/* events */
#define L2CAP_EV_CONNECTION_REQ		0x80
struct l2cap_connection_req_ev {
	u8_t chan_id;
	u16_t psm;
	u8_t address_type;
	u8_t address[6];
} __packed;

#define L2CAP_EV_CONNECTED		0x81
struct l2cap_connected_ev {
	u8_t chan_id;
	u16_t psm;
	u8_t address_type;
	u8_t address[6];
} __packed;

#define L2CAP_EV_DISCONNECTED		0x82
struct l2cap_disconnected_ev {
	u16_t result;
	u8_t chan_id;
	u16_t psm;
	u8_t address_type;
	u8_t address[6];
} __packed;

#define L2CAP_EV_DATA_RECEIVED		0x83
struct l2cap_data_received_ev {
	u8_t chan_id;
	u16_t data_length;
	u8_t data[0];
} __packed;

/* MESH Service */
/* commands */
#define MESH_READ_SUPPORTED_COMMANDS	0x01
struct mesh_read_supported_commands_rp {
	u8_t data[0];
} __packed;

#define MESH_OUT_BLINK			BIT(0)
#define MESH_OUT_BEEP			BIT(1)
#define MESH_OUT_VIBRATE		BIT(2)
#define MESH_OUT_DISPLAY_NUMBER		BIT(3)
#define MESH_OUT_DISPLAY_STRING		BIT(4)

#define MESH_IN_PUSH			BIT(0)
#define MESH_IN_TWIST			BIT(1)
#define MESH_IN_ENTER_NUMBER		BIT(2)
#define MESH_IN_ENTER_STRING		BIT(3)

#define MESH_CONFIG_PROVISIONING	0x02
struct mesh_config_provisioning_cmd {
	u8_t uuid[16];
	u8_t static_auth[16];
	u8_t out_size;
	u16_t out_actions;
	u8_t in_size;
	u16_t in_actions;
} __packed;

#define MESH_PROVISION_NODE		0x03
struct mesh_provision_node_cmd {
	u8_t net_key[16];
	u16_t net_key_idx;
	u8_t flags;
	u32_t iv_index;
	u32_t seq_num;
	u16_t addr;
	u8_t dev_key[16];
} __packed;

#define MESH_INIT			0x04
#define MESH_RESET			0x05
#define MESH_INPUT_NUMBER		0x06
struct mesh_input_number_cmd {
	u32_t number;
} __packed;

#define MESH_INPUT_STRING		0x07
struct mesh_input_string_cmd {
	u8_t string_len;
	u8_t string[0];
} __packed;

#define MESH_IVU_TEST_MODE		0x08
struct mesh_ivu_test_mode_cmd {
	u8_t enable;
} __packed;

#define MESH_IVU_TOGGLE_STATE			0x09

#define MESH_NET_SEND			0x0a
struct mesh_net_send_cmd {
	u8_t ttl;
	u16_t src;
	u16_t dst;
	u8_t payload_len;
	u8_t payload[0];
} __packed;

#define MESH_HEALTH_GENERATE_FAULTS	0x0b
struct mesh_health_generate_faults_rp {
	u8_t test_id;
	u8_t cur_faults_count;
	u8_t reg_faults_count;
	u8_t current_faults[0];
	u8_t registered_faults[0];
} __packed;

#define MESH_HEALTH_CLEAR_FAULTS	0x0c

#define MESH_LPN			0x0d
struct mesh_lpn_set_cmd {
	u8_t enable;
} __packed;

#define MESH_LPN_POLL			0x0e

#define MESH_MODEL_SEND			0x0f
struct mesh_model_send_cmd {
	u16_t src;
	u16_t dst;
	u8_t payload_len;
	u8_t payload[0];
} __packed;

#define MESH_LPN_SUBSCRIBE		0x10
struct mesh_lpn_subscribe_cmd {
	u16_t address;
} __packed;

#define MESH_LPN_UNSUBSCRIBE		0x11
struct mesh_lpn_unsubscribe_cmd {
	u16_t address;
} __packed;

#define MESH_RPL_CLEAR			0x12
#define MESH_PROXY_IDENTITY		0x13

/* events */
#define MESH_EV_OUT_NUMBER_ACTION	0x80
struct mesh_out_number_action_ev {
	u16_t action;
	u32_t number;
} __packed;

#define MESH_EV_OUT_STRING_ACTION	0x81
struct mesh_out_string_action_ev {
	u8_t string_len;
	u8_t string[0];
} __packed;

#define MESH_EV_IN_ACTION		0x82
struct mesh_in_action_ev {
	u16_t action;
	u8_t size;
} __packed;

#define MESH_EV_PROVISIONED		0x83

#define MESH_PROV_BEARER_PB_ADV		0x00
#define MESH_PROV_BEARER_PB_GATT	0x01
#define MESH_EV_PROV_LINK_OPEN		0x84
struct mesh_prov_link_open_ev {
	u8_t bearer;
} __packed;

#define MESH_EV_PROV_LINK_CLOSED	0x85
struct mesh_prov_link_closed_ev {
	u8_t bearer;
} __packed;

#define MESH_EV_NET_RECV		0x86
struct mesh_net_recv_ev {
	u8_t ttl;
	u8_t ctl;
	u16_t src;
	u16_t dst;
	u8_t payload_len;
	u8_t payload[0];
} __packed;

#define MESH_EV_INVALID_BEARER		0x87
struct mesh_invalid_bearer_ev {
	u8_t opcode;
} __packed;

#define MESH_EV_INCOMP_TIMER_EXP	0x88

void tester_init(void);
void tester_rsp(u8_t service, u8_t opcode, u8_t index, u8_t status);
void tester_send(u8_t service, u8_t opcode, u8_t index, u8_t *data,
		 size_t len);
void tester_send_buf(u8_t service, u8_t opcode, u8_t index,
		     struct os_mbuf *buf);

u8_t tester_init_gap(void);
u8_t tester_unregister_gap(void);
void tester_handle_gap(u8_t opcode, u8_t index, u8_t *data,
		       u16_t len);
u8_t tester_init_gatt(void);
u8_t tester_unregister_gatt(void);
void tester_handle_gatt(u8_t opcode, u8_t index, u8_t *data,
			u16_t len);
int tester_gatt_notify_rx_ev(u16_t conn_handle, u16_t attr_handle,
			     u8_t indication, struct os_mbuf *om);
int tester_gatt_subscribe_ev(u16_t conn_handle, u16_t attr_handle, u8_t reason,
			     u8_t prev_notify, u8_t cur_notify,
			     u8_t prev_indicate, u8_t cur_indicate);

#if MYNEWT_VAL(BLE_L2CAP_COC_MAX_NUM)
u8_t tester_init_l2cap(void);
u8_t tester_unregister_l2cap(void);
void tester_handle_l2cap(u8_t opcode, u8_t index, u8_t *data,
			 u16_t len);
#endif

#if MYNEWT_VAL(BLE_MESH)
u8_t tester_init_mesh(void);
u8_t tester_unregister_mesh(void);
void tester_handle_mesh(u8_t opcode, u8_t index, u8_t *data, u16_t len);
#endif /* MYNEWT_VAL(BLE_MESH) */

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
int gatt_svr_init(void);

#endif /* __BTTESTER_H__ */
