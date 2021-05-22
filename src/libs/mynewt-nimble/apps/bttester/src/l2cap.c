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

/* l2cap.c - Bluetooth L2CAP Tester */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "syscfg/syscfg.h"

#if MYNEWT_VAL(BLE_L2CAP_COC_MAX_NUM)

#include "console/console.h"
#include "host/ble_gap.h"
#include "host/ble_l2cap.h"

#include "../../../nimble/host/src/ble_l2cap_priv.h"

#include "bttester.h"

#define CONTROLLER_INDEX             0
#define CHANNELS                     MYNEWT_VAL(BLE_L2CAP_COC_MAX_NUM)
#define TESTER_COC_MTU               MYNEWT_VAL(BTTESTER_L2CAP_COC_MTU)
#define TESTER_COC_BUF_COUNT         (3 * MYNEWT_VAL(BLE_L2CAP_COC_MAX_NUM))

static os_membuf_t tester_sdu_coc_mem[
	OS_MEMPOOL_SIZE(TESTER_COC_BUF_COUNT, TESTER_COC_MTU)
];

struct os_mbuf_pool sdu_os_mbuf_pool;
static struct os_mempool sdu_coc_mbuf_mempool;

static struct channel {
	uint8_t chan_id; /* Internal number that identifies L2CAP channel. */
	uint8_t state;
	struct ble_l2cap_chan *chan;
} channels[CHANNELS];

static uint8_t recv_cb_buf[TESTER_COC_MTU + sizeof(struct l2cap_data_received_ev)];

static struct channel *get_free_channel(void)
{
	uint8_t i;
	struct channel *chan;

	for (i = 0; i < CHANNELS; i++) {
		if (channels[i].state) {
			continue;
		}

		chan = &channels[i];
		chan->chan_id = i;

		return chan;
	}

	return NULL;
}

struct channel *find_channel(struct ble_l2cap_chan *chan)
{
	int i;

	for (i = 0; i < CHANNELS; ++i) {
		if (channels[i].chan == chan) {
			return &channels[i];
		}
	}

	return NULL;
}

struct channel *get_channel(uint8_t chan_id)
{
	if (chan_id >= CHANNELS) {
		return NULL;
	}

	return &channels[chan_id];
}

static void
tester_l2cap_coc_recv(struct ble_l2cap_chan *chan, struct os_mbuf *sdu)
{
	SYS_LOG_DBG("LE CoC SDU received, chan: 0x%08lx, data len %d",
		    (uint32_t) chan, OS_MBUF_PKTLEN(sdu));

	os_mbuf_free_chain(sdu);
	sdu = os_mbuf_get_pkthdr(&sdu_os_mbuf_pool, 0);
	assert(sdu != NULL);

	ble_l2cap_recv_ready(chan, sdu);
}

static void recv_cb(uint16_t conn_handle, struct ble_l2cap_chan *chan,
		    struct os_mbuf *buf, void *arg)
{
	struct l2cap_data_received_ev *ev = (void *) recv_cb_buf;
	struct channel *channel = find_channel(chan);
	assert(channel != NULL);

	ev->chan_id = channel->chan_id;
	ev->data_length = OS_MBUF_PKTLEN(buf);

	if (ev->data_length > TESTER_COC_MTU) {
		SYS_LOG_ERR("Too large sdu received, truncating data");
		ev->data_length = TESTER_COC_MTU;
	}
	os_mbuf_copydata(buf, 0, ev->data_length, ev->data);

	tester_send(BTP_SERVICE_ID_L2CAP, L2CAP_EV_DATA_RECEIVED,
		    CONTROLLER_INDEX, recv_cb_buf, sizeof(*ev) + ev->data_length);

	tester_l2cap_coc_recv(chan, buf);
}

static void unstalled_cb(uint16_t conn_handle, struct ble_l2cap_chan *chan,
			 int status, void *arg)
{
	if (status) {
		tester_rsp(BTP_SERVICE_ID_L2CAP, L2CAP_SEND_DATA,
			   CONTROLLER_INDEX, BTP_STATUS_FAILED);
	} else {
		tester_rsp(BTP_SERVICE_ID_L2CAP, L2CAP_SEND_DATA,
			   CONTROLLER_INDEX, BTP_STATUS_SUCCESS);
	}
}

static void reconfigured_ev(uint16_t conn_handle, struct ble_l2cap_chan *chan,
			    struct ble_l2cap_chan_info *chan_info,
			    int status)
{
	struct l2cap_reconfigured_ev ev;
	struct channel *channel;

	if (status != 0) {
		return;
	}

	channel = find_channel(chan);
	assert(channel != NULL);

	ev.chan_id = channel->chan_id;
	ev.peer_mtu = chan_info->peer_coc_mtu;
	ev.peer_mps = chan_info->peer_l2cap_mtu;
	ev.our_mtu = chan_info->our_coc_mtu;
	ev.our_mps = chan_info->our_l2cap_mtu;

	tester_send(BTP_SERVICE_ID_L2CAP, L2CAP_EV_RECONFIGURED,
		    CONTROLLER_INDEX, (uint8_t *) &ev, sizeof(ev));
}

static void connected_cb(uint16_t conn_handle, struct ble_l2cap_chan *chan,
			 struct ble_l2cap_chan_info *chan_info, void *arg)
{
	struct l2cap_connected_ev ev;
	struct ble_gap_conn_desc desc;
	struct channel *channel = find_channel(chan);

	if (channel == NULL) {
		channel = get_free_channel();
	}

	ev.chan_id = channel->chan_id;
	ev.psm = chan_info->psm;
	ev.peer_mtu = chan_info->peer_coc_mtu;
	ev.peer_mps = chan_info->peer_l2cap_mtu;
	ev.our_mtu = chan_info->our_coc_mtu;
	ev.our_mps = chan_info->our_l2cap_mtu;
	channel->state = 1;
	channel->chan = chan;

	if (!ble_gap_conn_find(conn_handle, &desc)) {
		ev.address_type = desc.peer_ota_addr.type;
		memcpy(ev.address, desc.peer_ota_addr.val,
		       sizeof(ev.address));
	}

	tester_send(BTP_SERVICE_ID_L2CAP, L2CAP_EV_CONNECTED, CONTROLLER_INDEX,
		    (uint8_t *) &ev, sizeof(ev));
}

static void disconnected_cb(uint16_t conn_handle, struct ble_l2cap_chan *chan,
			    struct ble_l2cap_chan_info *chan_info, void *arg)
{
	struct l2cap_disconnected_ev ev;
	struct ble_gap_conn_desc desc;
	struct channel *channel;

	memset(&ev, 0, sizeof(struct l2cap_disconnected_ev));

	channel = find_channel(chan);
	assert(channel != NULL);

	channel->state = 0;
	channel->chan = chan;
	ev.chan_id = channel->chan_id;
	ev.psm = chan_info->psm;

	if (!ble_gap_conn_find(conn_handle, &desc)) {
		ev.address_type = desc.peer_ota_addr.type;
		memcpy(ev.address, desc.peer_ota_addr.val,
		       sizeof(ev.address));
	}

	tester_send(BTP_SERVICE_ID_L2CAP, L2CAP_EV_DISCONNECTED,
		    CONTROLLER_INDEX, (uint8_t *) &ev, sizeof(ev));
}

static int accept_cb(uint16_t conn_handle, uint16_t peer_mtu,
		     struct ble_l2cap_chan *chan)
{
	struct os_mbuf *sdu_rx;

	SYS_LOG_DBG("LE CoC accepting, chan: 0x%08lx, peer_mtu %d",
		    (uint32_t) chan, peer_mtu);

	sdu_rx = os_mbuf_get_pkthdr(&sdu_os_mbuf_pool, 0);
	if (!sdu_rx) {
		return BLE_HS_ENOMEM;
	}

	ble_l2cap_recv_ready(chan, sdu_rx);

	return 0;
}

static int
tester_l2cap_event(struct ble_l2cap_event *event, void *arg)
{
	struct ble_l2cap_chan_info chan_info;
	int accept_response;

	switch (event->type) {
		case BLE_L2CAP_EVENT_COC_CONNECTED:
		if (ble_l2cap_get_chan_info(event->connect.chan, &chan_info)) {
			assert(0);
		}

		if (event->connect.status) {
			console_printf("LE COC error: %d\n", event->connect.status);
			disconnected_cb(event->connect.conn_handle,
					event->connect.chan, &chan_info, arg);
			return 0;
		}

		console_printf("LE COC connected, conn: %d, chan: 0x%08lx, "
			       "psm: 0x%02x, scid: 0x%04x, dcid: 0x%04x, "
			       "our_mps: %d, our_mtu: %d, peer_mps: %d, "
			       "peer_mtu: %d\n", event->connect.conn_handle,
			       (uint32_t) event->connect.chan, chan_info.psm,
			       chan_info.scid, chan_info.dcid,
			       chan_info.our_l2cap_mtu, chan_info.our_coc_mtu,
			       chan_info.peer_l2cap_mtu, chan_info.peer_coc_mtu);

		connected_cb(event->connect.conn_handle,
			     event->connect.chan, &chan_info, arg);

		return 0;
	case BLE_L2CAP_EVENT_COC_DISCONNECTED:
		if (ble_l2cap_get_chan_info(event->disconnect.chan,
					    &chan_info)) {
			assert(0);
		}
		console_printf("LE CoC disconnected, chan: 0x%08lx\n",
			       (uint32_t) event->disconnect.chan);

		disconnected_cb(event->disconnect.conn_handle,
				event->disconnect.chan, &chan_info, arg);
		return 0;
	case BLE_L2CAP_EVENT_COC_ACCEPT:
		accept_response = POINTER_TO_INT(arg);
		if (accept_response) {
			return accept_response;
		}

		console_printf("LE CoC accept, chan: 0x%08lx, handle: %u, sdu_size: %u\n",
			       (uint32_t) event->accept.chan,
			       event->accept.conn_handle,
			       event->accept.peer_sdu_size);

		return accept_cb(event->accept.conn_handle,
				 event->accept.peer_sdu_size,
				 event->accept.chan);

	case BLE_L2CAP_EVENT_COC_DATA_RECEIVED:
		console_printf("LE CoC data received, chan: 0x%08lx, handle: %u, sdu_len: %u\n",
			       (uint32_t) event->receive.chan,
			       event->receive.conn_handle,
			       OS_MBUF_PKTLEN(event->receive.sdu_rx));

		recv_cb(event->receive.conn_handle, event->receive.chan,
			event->receive.sdu_rx, arg);
		return 0;
	case BLE_L2CAP_EVENT_COC_TX_UNSTALLED:
		console_printf("LE CoC tx unstalled, chan: 0x%08lx, handle: %u, status: %d\n",
			       (uint32_t) event->tx_unstalled.chan,
			       event->tx_unstalled.conn_handle,
			       event->tx_unstalled.status);

		unstalled_cb(event->tx_unstalled.conn_handle,
			     event->tx_unstalled.chan,
			     event->tx_unstalled.status, arg);
		return 0;
	case BLE_L2CAP_EVENT_COC_RECONFIG_COMPLETED:
		if (ble_l2cap_get_chan_info(event->reconfigured.chan,
					    &chan_info)) {
			assert(0);
		}
		console_printf("LE CoC reconfigure completed status 0x%02x, "
			       "chan: 0x%08lx\n", event->reconfigured.status,
			       (uint32_t) event->reconfigured.chan);

		if (event->reconfigured.status == 0) {
			console_printf("\t our_mps: %d our_mtu %d\n",
				       chan_info.our_l2cap_mtu, chan_info.our_coc_mtu);
		}

		reconfigured_ev(event->reconfigured.conn_handle,
				event->reconfigured.chan,
				&chan_info,
				event->reconfigured.status);
		return 0;
	case BLE_L2CAP_EVENT_COC_PEER_RECONFIGURED:
		if (ble_l2cap_get_chan_info(event->reconfigured.chan,
					    &chan_info)) {
			assert(0);
		}
		console_printf("LE CoC peer reconfigured status 0x%02x, "
			       "chan: 0x%08lx\n", event->reconfigured.status,
			       (uint32_t) event->reconfigured.chan);

		if (event->reconfigured.status == 0) {
			console_printf("\t peer_mps: %d peer_mtu %d\n",
				       chan_info.peer_l2cap_mtu, chan_info.peer_coc_mtu);
		}

		reconfigured_ev(event->reconfigured.conn_handle,
				event->reconfigured.chan,
				&chan_info,
				event->reconfigured.status);
		return 0;
	default:
		return 0;
	}
}

static void connect(uint8_t *data, uint16_t len)
{
	const struct l2cap_connect_cmd *cmd = (void *) data;
	uint8_t rp_buf[sizeof(struct l2cap_connect_rp) + cmd->num];
	struct l2cap_connect_rp *rp = (void *) rp_buf;
	struct ble_gap_conn_desc desc;
	struct channel *chan;
	struct os_mbuf *sdu_rx[cmd->num];
	ble_addr_t *addr = (void *) data;
	uint16_t mtu = htole16(cmd->mtu);
	int rc;
	int i;

	SYS_LOG_DBG("connect: type: %d addr: %s", addr->type, bt_hex(addr->val, 6));

	if (mtu == 0 || mtu > TESTER_COC_MTU) {
		mtu = TESTER_COC_MTU;
	}

	rc = ble_gap_conn_find_by_addr(addr, &desc);
	if (rc) {
		SYS_LOG_ERR("GAP conn find failed");
		goto fail;
	}

	rp->num = cmd->num;

	for (i = 0; i < cmd->num; i++) {
		chan = get_free_channel();
		if (!chan) {
			SYS_LOG_ERR("No free channels");
			goto fail;
		}

		rp->chan_ids[i] = chan->chan_id;

		sdu_rx[i] = os_mbuf_get_pkthdr(&sdu_os_mbuf_pool, 0);
		if (sdu_rx[i] == NULL) {
			SYS_LOG_ERR("Failed to alloc buf");
			goto fail;
		}
	}

	if (cmd->num == 1) {
		rc = ble_l2cap_connect(desc.conn_handle, htole16(cmd->psm),
				       mtu, sdu_rx[0],
				       tester_l2cap_event, NULL);
	} else if (cmd->num > 1) {
		rc = ble_l2cap_enhanced_connect(desc.conn_handle,
						htole16(cmd->psm), mtu,
						cmd->num, sdu_rx,
						tester_l2cap_event, NULL);
	} else {
		SYS_LOG_ERR("Invalid 'num' parameter value");
		goto fail;
	}

	if (rc) {
		SYS_LOG_ERR("L2CAP connect failed\n");
		goto fail;
	}

	tester_send(BTP_SERVICE_ID_L2CAP, L2CAP_CONNECT, CONTROLLER_INDEX,
		    (uint8_t *) rp, sizeof(rp_buf));

	return;

fail:
	tester_rsp(BTP_SERVICE_ID_L2CAP, L2CAP_CONNECT, CONTROLLER_INDEX,
		   BTP_STATUS_FAILED);
}

static void disconnect(const uint8_t *data, uint16_t len)
{
	const struct l2cap_disconnect_cmd *cmd = (void *) data;
	struct channel *chan;
	uint8_t status;
	int err;

	SYS_LOG_DBG("");

	chan = get_channel(cmd->chan_id);
	assert(chan != NULL);

	err = ble_l2cap_disconnect(chan->chan);
	if (err) {
		status = BTP_STATUS_FAILED;
		goto rsp;
	}

	status = BTP_STATUS_SUCCESS;

rsp:
	tester_rsp(BTP_SERVICE_ID_L2CAP, L2CAP_DISCONNECT, CONTROLLER_INDEX,
		   status);
}

static void send_data(const uint8_t *data, uint16_t len)
{
	const struct l2cap_send_data_cmd *cmd = (void *) data;
	struct os_mbuf *sdu_tx = NULL;
	int rc;
	uint16_t data_len = sys_le16_to_cpu(cmd->data_len);
	struct channel *chan = get_channel(cmd->chan_id);

	SYS_LOG_DBG("cmd->chan_id=%d", cmd->chan_id);

	if (!chan) {
		SYS_LOG_ERR("Invalid channel\n");
		goto fail;
	}

	/* FIXME: For now, fail if data length exceeds buffer length */
	if (data_len > TESTER_COC_MTU) {
		SYS_LOG_ERR("Data length exceeds buffer length");
		goto fail;
	}

	sdu_tx = os_mbuf_get_pkthdr(&sdu_os_mbuf_pool, 0);
	if (sdu_tx == NULL) {
		SYS_LOG_ERR("No memory in the test sdu pool\n");
		goto fail;
	}

	os_mbuf_append(sdu_tx, cmd->data, data_len);

	/* ble_l2cap_send takes ownership of the sdu */
	rc = ble_l2cap_send(chan->chan, sdu_tx);
	if (rc == 0 || rc == BLE_HS_ESTALLED) {
		tester_rsp(BTP_SERVICE_ID_L2CAP, L2CAP_SEND_DATA, CONTROLLER_INDEX,
			   BTP_STATUS_SUCCESS);
		return;
	}

	SYS_LOG_ERR("Unable to send data: %d", rc);
	os_mbuf_free_chain(sdu_tx);

fail:
	tester_rsp(BTP_SERVICE_ID_L2CAP, L2CAP_SEND_DATA, CONTROLLER_INDEX,
		   BTP_STATUS_FAILED);
}

static int
l2cap_coc_err2hs_err(uint16_t coc_err)
{
	switch (coc_err) {
		case BLE_L2CAP_COC_ERR_UNKNOWN_LE_PSM:
			return BLE_HS_ENOTSUP;
		case BLE_L2CAP_COC_ERR_NO_RESOURCES:
			return BLE_HS_ENOMEM;
		case BLE_L2CAP_COC_ERR_INSUFFICIENT_AUTHEN:
			return BLE_HS_EAUTHEN;
		case BLE_L2CAP_COC_ERR_INSUFFICIENT_AUTHOR:
			return BLE_HS_EAUTHOR;
		case BLE_L2CAP_COC_ERR_INSUFFICIENT_ENC:
			return BLE_HS_EENCRYPT;
		case BLE_L2CAP_COC_ERR_INSUFFICIENT_KEY_SZ:
			return BLE_HS_EENCRYPT_KEY_SZ;
		case BLE_L2CAP_COC_ERR_UNACCEPTABLE_PARAMETERS:
			return BLE_HS_EINVAL;
		default:
			return 0;
	}
}


static void listen(const uint8_t *data, uint16_t len)
{
	const struct l2cap_listen_cmd *cmd = (void *) data;
	uint16_t mtu = htole16(cmd->mtu);
	uint16_t rsp = htole16(cmd->response);
	int rc;

	SYS_LOG_DBG("");

	if (mtu == 0 || mtu > TESTER_COC_MTU) {
		mtu = TESTER_COC_MTU;
	}

	rsp = l2cap_coc_err2hs_err(rsp);

	/* TODO: Handle cmd->transport flag */
	rc = ble_l2cap_create_server(cmd->psm, mtu, tester_l2cap_event,
				     INT_TO_POINTER(rsp));
	if (rc) {
		goto fail;
	}

	tester_rsp(BTP_SERVICE_ID_L2CAP, L2CAP_LISTEN, CONTROLLER_INDEX,
		   BTP_STATUS_SUCCESS);
	return;

fail:
	tester_rsp(BTP_SERVICE_ID_L2CAP, L2CAP_LISTEN, CONTROLLER_INDEX,
		   BTP_STATUS_FAILED);
}

static void reconfigure(const uint8_t *data, uint16_t len)
{
	const struct l2cap_reconfigure_cmd *cmd = (void *) data;
	uint16_t mtu = htole16(cmd->mtu);
	struct ble_gap_conn_desc desc;
	ble_addr_t *addr = (void *) data;
	struct ble_l2cap_chan *chans[cmd->num];
	struct channel *channel;
	int rc;
	int i;

	SYS_LOG_DBG("");

	if (mtu == 0 || mtu > TESTER_COC_MTU) {
		mtu = TESTER_COC_MTU;
	}

	rc = ble_gap_conn_find_by_addr(addr, &desc);
	if (rc) {
		SYS_LOG_ERR("GAP conn find failed");
		goto fail;
	}

	for (i = 0; i < cmd->num; ++i) {
		channel = get_channel(cmd->idxs[i]);
		if (channel == NULL) {
			goto fail;
		}
		chans[i] = channel->chan;
	}

	rc = ble_l2cap_reconfig(chans, cmd->num, mtu);
	if (rc) {
		goto fail;
	}

	tester_rsp(BTP_SERVICE_ID_L2CAP, L2CAP_RECONFIGURE, CONTROLLER_INDEX,
		   BTP_STATUS_SUCCESS);
	return;

fail:
	tester_rsp(BTP_SERVICE_ID_L2CAP, L2CAP_RECONFIGURE, CONTROLLER_INDEX,
		   BTP_STATUS_FAILED);
}

static void supported_commands(uint8_t *data, uint16_t len)
{
	uint8_t cmds[1];
	struct l2cap_read_supported_commands_rp *rp = (void *) cmds;

	memset(cmds, 0, sizeof(cmds));

	tester_set_bit(cmds, L2CAP_READ_SUPPORTED_COMMANDS);
	tester_set_bit(cmds, L2CAP_CONNECT);
	tester_set_bit(cmds, L2CAP_DISCONNECT);
	tester_set_bit(cmds, L2CAP_LISTEN);
	tester_set_bit(cmds, L2CAP_SEND_DATA);
	tester_set_bit(cmds, L2CAP_RECONFIGURE);

	tester_send(BTP_SERVICE_ID_L2CAP, L2CAP_READ_SUPPORTED_COMMANDS,
		    CONTROLLER_INDEX, (uint8_t *) rp, sizeof(cmds));
}

void tester_handle_l2cap(uint8_t opcode, uint8_t index, uint8_t *data,
			 uint16_t len)
{
	switch (opcode) {
	case L2CAP_READ_SUPPORTED_COMMANDS:
		supported_commands(data, len);
		return;
	case L2CAP_CONNECT:
		connect(data, len);
		return;
	case L2CAP_DISCONNECT:
		disconnect(data, len);
		return;
	case L2CAP_SEND_DATA:
		send_data(data, len);
		return;
	case L2CAP_LISTEN:
		listen(data, len);
		return;
	case L2CAP_RECONFIGURE:
		reconfigure(data, len);
		return;
	default:
		tester_rsp(BTP_SERVICE_ID_L2CAP, opcode, index,
			   BTP_STATUS_UNKNOWN_CMD);
		return;
	}
}

uint8_t tester_init_l2cap(void)
{
	int rc;

	/* For testing we want to support all the available channels */
	rc = os_mempool_init(&sdu_coc_mbuf_mempool, TESTER_COC_BUF_COUNT,
			     TESTER_COC_MTU, tester_sdu_coc_mem,
			     "tester_coc_sdu_pool");
	assert(rc == 0);

	rc = os_mbuf_pool_init(&sdu_os_mbuf_pool, &sdu_coc_mbuf_mempool,
			       TESTER_COC_MTU, TESTER_COC_BUF_COUNT);
	assert(rc == 0);

	return BTP_STATUS_SUCCESS;
}

uint8_t tester_unregister_l2cap(void)
{
	return BTP_STATUS_SUCCESS;
}

#endif
