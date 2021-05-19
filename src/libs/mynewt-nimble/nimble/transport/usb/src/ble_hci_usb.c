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

#include <assert.h>
#include <stddef.h>
#include "syscfg/syscfg.h"
#include "sysinit/sysinit.h"
#include "os/os.h"
#include "mem/mem.h"

#include "nimble/ble.h"
#include "nimble/ble_hci_trans.h"
#include "nimble/hci_common.h"

#include <class/bth/bth_device.h>

/*
 * The MBUF payload size must accommodate the HCI data header size plus the
 * maximum ACL data packet length. The ACL block size is the size of the
 * mbufs we will allocate.
 */
#define ACL_BLOCK_SIZE  OS_ALIGN(MYNEWT_VAL(BLE_ACL_BUF_SIZE) \
                                 + BLE_MBUF_MEMBLOCK_OVERHEAD \
                                 + BLE_HCI_DATA_HDR_SZ, OS_ALIGNMENT)

struct usb_ble_hci_pool_cmd {
    uint8_t cmd[BLE_HCI_TRANS_CMD_SZ];
    bool allocated;
};

/* (Pseudo)pool for HCI commands */
static struct usb_ble_hci_pool_cmd usb_ble_hci_pool_cmd;

static ble_hci_trans_rx_cmd_fn *ble_hci_usb_rx_cmd_ll_cb;
static void *ble_hci_usb_rx_cmd_ll_arg;

static ble_hci_trans_rx_acl_fn *ble_hci_usb_rx_acl_ll_cb;
static void *ble_hci_usb_rx_acl_ll_arg;

static struct os_mempool ble_hci_usb_evt_hi_pool;
static os_membuf_t ble_hci_usb_evt_hi_buf[
    OS_MEMPOOL_SIZE(MYNEWT_VAL(BLE_HCI_EVT_HI_BUF_COUNT),
                    MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE))
];

static struct os_mempool ble_hci_usb_evt_lo_pool;
static os_membuf_t ble_hci_usb_evt_lo_buf[
    OS_MEMPOOL_SIZE(MYNEWT_VAL(BLE_HCI_EVT_LO_BUF_COUNT),
                    MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE))
];

static uint8_t ble_hci_pool_acl_mempool_buf[
    OS_MEMPOOL_BYTES(MYNEWT_VAL(BLE_ACL_BUF_COUNT),
                     ACL_BLOCK_SIZE)];
static struct os_mempool ble_hci_pool_acl_mempool;
static struct os_mbuf_pool ble_hci_pool_acl_mbuf_pool;

static struct os_mbuf *incoming_acl_data;

static struct os_mbuf *
ble_hci_trans_acl_buf_alloc(void)
{
    struct os_mbuf *m;

    m = os_mbuf_get_pkthdr(&ble_hci_pool_acl_mbuf_pool,
                           sizeof(struct ble_mbuf_hdr));
    return m;
}

void
ble_hci_trans_cfg_ll(ble_hci_trans_rx_cmd_fn *cmd_cb,
                     void *cmd_arg,
                     ble_hci_trans_rx_acl_fn *acl_cb,
                     void *acl_arg)
{
    ble_hci_usb_rx_cmd_ll_cb = cmd_cb;
    ble_hci_usb_rx_cmd_ll_arg = cmd_arg;
    ble_hci_usb_rx_acl_ll_cb = acl_cb;
    ble_hci_usb_rx_acl_ll_arg = acl_arg;
}

#define BLE_HCI_USB_EVT_COUNT  \
    (MYNEWT_VAL(BLE_HCI_EVT_HI_BUF_COUNT) + MYNEWT_VAL(BLE_HCI_EVT_LO_BUF_COUNT))

/**
 * A packet to be sent over the USB.  This can be a command, an event, or ACL
 * data.
 */
struct ble_hci_pkt {
    STAILQ_ENTRY(ble_hci_pkt) next;
    void *data;
};

static struct os_mempool ble_hci_pkt_pool;
static os_membuf_t ble_hci_pkt_buf[
    OS_MEMPOOL_SIZE(BLE_HCI_USB_EVT_COUNT + 1 +
                    MYNEWT_VAL(BLE_HCI_ACL_OUT_COUNT),
                    sizeof(struct ble_hci_pkt))];

struct tx_queue {
    STAILQ_HEAD(, ble_hci_pkt) queue;
};

static struct tx_queue ble_hci_tx_acl_queue = {STAILQ_HEAD_INITIALIZER(ble_hci_tx_acl_queue.queue)};
static struct tx_queue ble_hci_tx_evt_queue = { STAILQ_HEAD_INITIALIZER(ble_hci_tx_evt_queue.queue) };

/*
 * TinyUSB callbacks.
 */
void
tud_bt_acl_data_sent_cb(uint16_t sent_bytes)
{
    struct os_mbuf *om;
    struct ble_hci_pkt *curr_acl;
    struct ble_hci_pkt *next_acl;
    os_sr_t sr;

    OS_ENTER_CRITICAL(sr);
    curr_acl = STAILQ_FIRST(&ble_hci_tx_acl_queue.queue);
    OS_EXIT_CRITICAL(sr);

    assert(curr_acl != NULL);
    om = curr_acl->data;
    assert(om != NULL && om->om_len >= sent_bytes);
    os_mbuf_adj(om, sent_bytes);

    while (om != NULL && om->om_len == 0) {
        curr_acl->data = SLIST_NEXT(om, om_next);
        os_mbuf_free(om);
        om = curr_acl->data;
    }

    if (om == NULL) {
        OS_ENTER_CRITICAL(sr);
        STAILQ_REMOVE_HEAD(&ble_hci_tx_acl_queue.queue, next);
        next_acl = STAILQ_FIRST(&ble_hci_tx_acl_queue.queue);
        OS_EXIT_CRITICAL(sr);
        os_memblock_put(&ble_hci_pkt_pool, curr_acl);
        if (next_acl != NULL) {
            om = next_acl->data;
        }
    }

    if (om != NULL) {
        tud_bt_acl_data_send(om->om_data, om->om_len);
    }
}

void
tud_bt_event_sent_cb(uint16_t sent_bytes)
{
    struct ble_hci_pkt *curr_evt;
    struct ble_hci_pkt *next_evt;
    uint8_t *hci_ev;
    os_sr_t sr;

    OS_ENTER_CRITICAL(sr);
    curr_evt = STAILQ_FIRST(&ble_hci_tx_evt_queue.queue);
    OS_EXIT_CRITICAL(sr);
    assert(curr_evt != NULL);
    hci_ev = curr_evt->data;

    assert(hci_ev != NULL && hci_ev[1] + sizeof(struct ble_hci_ev) == sent_bytes);

    ble_hci_trans_buf_free(hci_ev);

    OS_ENTER_CRITICAL(sr);
    STAILQ_REMOVE_HEAD(&ble_hci_tx_evt_queue.queue, next);
    next_evt = STAILQ_FIRST(&ble_hci_tx_evt_queue.queue);
    OS_EXIT_CRITICAL(sr);
    os_memblock_put(&ble_hci_pkt_pool, curr_evt);

    if (next_evt != NULL) {
        hci_ev = next_evt->data;
        tud_bt_event_send(hci_ev, hci_ev[1] + sizeof(struct ble_hci_ev));
    }
}

void
tud_bt_acl_data_received_cb(void *acl_data, uint16_t data_len)
{
    uint8_t *data;
    uint32_t len;
    struct os_mbuf *om = incoming_acl_data;
    int rc;

    if (om == NULL) {
        om = ble_hci_trans_acl_buf_alloc();
        assert(om != NULL);
    }
    assert(om->om_len + data_len <= MYNEWT_VAL(BLE_ACL_BUF_SIZE) + BLE_HCI_DATA_HDR_SZ);

    os_mbuf_append(om, acl_data, data_len);
    incoming_acl_data = om;
    if (om->om_len > BLE_HCI_DATA_HDR_SZ) {
        data = incoming_acl_data->om_data;
        len = data[2] + (data[3] << 8) + BLE_HCI_DATA_HDR_SZ;
        if (incoming_acl_data->om_len >= len) {
            incoming_acl_data = NULL;
            rc = ble_hci_usb_rx_acl_ll_cb(om, ble_hci_usb_rx_acl_ll_arg);
            (void)rc;
        }
    }
}

void
tud_bt_hci_cmd_cb(void *hci_cmd, size_t cmd_len)
{
    uint8_t *buf;
    int rc = -1;

    assert(ble_hci_usb_rx_cmd_ll_cb);
    if (ble_hci_usb_rx_cmd_ll_cb) {
        buf = ble_hci_trans_buf_alloc(BLE_HCI_TRANS_BUF_CMD);
        assert(buf != NULL);
        memcpy(buf, hci_cmd, cmd_len);

        rc = ble_hci_usb_rx_cmd_ll_cb(buf, ble_hci_usb_rx_cmd_ll_arg);
    }

    if (rc != 0) {
        ble_hci_trans_buf_free(buf);
    }
}

static int
ble_hci_trans_ll_tx(struct tx_queue *queue, struct os_mbuf *om)
{
    struct ble_hci_pkt *pkt;
    os_sr_t sr;
    bool first;

    /* If this packet is zero length, just free it */
    if (OS_MBUF_PKTLEN(om) == 0) {
        os_mbuf_free_chain(om);
        return 0;
    }

    pkt = os_memblock_get(&ble_hci_pkt_pool);
    if (pkt == NULL) {
        os_mbuf_free_chain(om);
        return BLE_ERR_MEM_CAPACITY;
    }

    pkt->data = om;
    OS_ENTER_CRITICAL(sr);
    first = STAILQ_EMPTY(&queue->queue);
    STAILQ_INSERT_TAIL(&queue->queue, pkt, next);
    OS_EXIT_CRITICAL(sr);
    if (first) {
        tud_bt_acl_data_send(om->om_data, om->om_len);
    }

    return 0;
}

int
ble_hci_trans_ll_acl_tx(struct os_mbuf *om)
{
    return ble_hci_trans_ll_tx(&ble_hci_tx_acl_queue, om);
}

int
ble_hci_trans_ll_evt_tx(uint8_t *hci_ev)
{
    struct ble_hci_pkt *pkt;
    os_sr_t sr;
    bool first;

    assert(hci_ev != NULL);

    pkt = os_memblock_get(&ble_hci_pkt_pool);
    if (pkt == NULL) {
        ble_hci_trans_buf_free(hci_ev);
        return BLE_ERR_MEM_CAPACITY;
    }

    pkt->data = hci_ev;
    OS_ENTER_CRITICAL(sr);
    first = STAILQ_EMPTY(&ble_hci_tx_evt_queue.queue);
    STAILQ_INSERT_TAIL(&ble_hci_tx_evt_queue.queue, pkt, next);
    OS_EXIT_CRITICAL(sr);
    if (first) {
        tud_bt_event_send(hci_ev, hci_ev[1] + sizeof(struct ble_hci_ev));
    }

    return 0;
}

uint8_t *
ble_hci_trans_buf_alloc(int type)
{
    uint8_t *buf;

    switch (type) {
    case BLE_HCI_TRANS_BUF_CMD:
        assert(!usb_ble_hci_pool_cmd.allocated);
        usb_ble_hci_pool_cmd.allocated = 1;
        buf = usb_ble_hci_pool_cmd.cmd;
        break;

    case BLE_HCI_TRANS_BUF_EVT_HI:
        buf = os_memblock_get(&ble_hci_usb_evt_hi_pool);
        if (buf == NULL) {
            /* If no high-priority event buffers remain, try to grab a
             * low-priority one.
             */
            buf = ble_hci_trans_buf_alloc(BLE_HCI_TRANS_BUF_EVT_LO);
        }
        break;

    case BLE_HCI_TRANS_BUF_EVT_LO:
        buf = os_memblock_get(&ble_hci_usb_evt_lo_pool);
        break;

    default:
        assert(0);
        buf = NULL;
    }

    return buf;
}

void
ble_hci_trans_buf_free(uint8_t *buf)
{
    int rc;

    /* XXX: this may look a bit odd, but the controller uses the command
     * buffer to send back the command complete/status as an immediate
     * response to the command. This was done to insure that the controller
     * could always send back one of these events when a command was received.
     * Thus, we check to see which pool the buffer came from so we can free
     * it to the appropriate pool
     */
    if (os_memblock_from(&ble_hci_usb_evt_hi_pool, buf)) {
        rc = os_memblock_put(&ble_hci_usb_evt_hi_pool, buf);
        assert(rc == 0);
    } else if (os_memblock_from(&ble_hci_usb_evt_lo_pool, buf)) {
        rc = os_memblock_put(&ble_hci_usb_evt_lo_pool, buf);
        assert(rc == 0);
    } else {
        assert(usb_ble_hci_pool_cmd.allocated);
        usb_ble_hci_pool_cmd.allocated = 0;
    }
    (void)rc;
}

int
ble_hci_trans_reset(void)
{
    return 0;
}

void
ble_hci_usb_init(void)
{
    int rc;

    /* Ensure this function only gets called by sysinit. */
    SYSINIT_ASSERT_ACTIVE();

    rc = mem_init_mbuf_pool(ble_hci_pool_acl_mempool_buf, &ble_hci_pool_acl_mempool, &ble_hci_pool_acl_mbuf_pool,
                            MYNEWT_VAL(BLE_ACL_BUF_COUNT), ACL_BLOCK_SIZE, "ble_hci_acl");
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = os_mempool_init(&ble_hci_usb_evt_hi_pool,
                         MYNEWT_VAL(BLE_HCI_EVT_HI_BUF_COUNT),
                         MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE),
                         ble_hci_usb_evt_hi_buf,
                         "ble_hci_usb_evt_hi_pool");
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = os_mempool_init(&ble_hci_usb_evt_lo_pool,
                         MYNEWT_VAL(BLE_HCI_EVT_LO_BUF_COUNT),
                         MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE),
                         ble_hci_usb_evt_lo_buf,
                         "ble_hci_usb_evt_lo_pool");
    SYSINIT_PANIC_ASSERT(rc == 0);

    /*
     * Create memory pool of packet list nodes. NOTE: the number of these
     * buffers should be, at least, the total number of event buffers (hi
     * and lo), the number of command buffers (currently 1) and the total
     * number of buffers that the controller could possibly hand to the host.
     */
    rc = os_mempool_init(&ble_hci_pkt_pool,
                         BLE_HCI_USB_EVT_COUNT + 1 +
                         MYNEWT_VAL(BLE_HCI_ACL_OUT_COUNT),
                         sizeof (struct ble_hci_pkt),
                         ble_hci_pkt_buf,
                         "ble_hci_usb_pkt_pool");
    SYSINIT_PANIC_ASSERT(rc == 0);
}
