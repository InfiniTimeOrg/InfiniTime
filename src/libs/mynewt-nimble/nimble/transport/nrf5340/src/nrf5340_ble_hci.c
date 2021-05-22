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
#include <string.h>
#include <os/mynewt.h>
#include <nimble/ble.h>
#include <nimble/ble_hci_trans.h>
#include <nimble/hci_common.h>
#include <ipc_nrf5340/ipc_nrf5340.h>

#define HCI_PKT_NONE    0x00
#define HCI_PKT_CMD     0x01
#define HCI_PKT_ACL     0x02
#define HCI_PKT_EVT     0x04

#define POOL_ACL_BLOCK_SIZE OS_ALIGN(MYNEWT_VAL(BLE_ACL_BUF_SIZE) +     \
                                     BLE_MBUF_MEMBLOCK_OVERHEAD +       \
                                     BLE_HCI_DATA_HDR_SZ, OS_ALIGNMENT)

#if MYNEWT_VAL(BLE_CONTROLLER)
#define IPC_TX_CHANNEL 0
#define IPC_RX_CHANNEL 1
#endif

#if MYNEWT_VAL(BLE_HOST)
#define IPC_TX_CHANNEL 1
#define IPC_RX_CHANNEL 0
#endif

struct nrf5340_ble_hci_api {
#if MYNEWT_VAL(BLE_CONTROLLER)
    ble_hci_trans_rx_cmd_fn *cmd_cb;
    void *cmd_arg;
#endif
#if MYNEWT_VAL(BLE_HOST)
    ble_hci_trans_rx_cmd_fn *evt_cb;
    void *evt_arg;
#endif
    ble_hci_trans_rx_acl_fn *acl_cb;
    void *acl_arg;
};

struct nrf5340_ble_hci_rx_data {
    uint8_t type;
    uint8_t hdr[4];
    uint16_t len;
    uint16_t expected_len;
    union {
        uint8_t *buf;
        struct os_mbuf *om;
    };
};

struct nrf5340_ble_hci_pool_cmd {
    uint8_t cmd[BLE_HCI_TRANS_CMD_SZ];
    bool allocated;
};

/* (Pseudo)pool for HCI commands */
static struct nrf5340_ble_hci_pool_cmd nrf5340_ble_hci_pool_cmd;

/* Pools for HCI events (high and low priority) */
static uint8_t nrf5340_ble_hci_pool_evt_hi_buf[OS_MEMPOOL_BYTES(
                                            MYNEWT_VAL(BLE_HCI_EVT_HI_BUF_COUNT),
                                            MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE))];
static struct os_mempool nrf5340_ble_hci_pool_evt_hi;
static uint8_t nrf5340_ble_hci_pool_evt_lo_buf[OS_MEMPOOL_BYTES(
                                            MYNEWT_VAL(BLE_HCI_EVT_LO_BUF_COUNT),
                                            MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE))];
static struct os_mempool nrf5340_ble_hci_pool_evt_lo;

/* Pool for ACL data */
static uint8_t nrf5340_ble_hci_pool_acl_buf[OS_MEMPOOL_BYTES(
                                            MYNEWT_VAL(BLE_ACL_BUF_COUNT),
                                            POOL_ACL_BLOCK_SIZE)];
static struct os_mempool nrf5340_ble_hci_pool_acl;
static struct os_mbuf_pool nrf5340_ble_hci_pool_acl_mbuf;

/* Interface to host/ll */
static struct nrf5340_ble_hci_api nrf5340_ble_hci_api;

/* State of RX currently in progress (needs to reassemble frame) */
static struct nrf5340_ble_hci_rx_data nrf5340_ble_hci_rx_data;

int
ble_hci_trans_reset(void)
{
    /* XXX Should we do something with RF and/or BLE core? */
    return 0;
}

static int
ble_hci_trans_acl_tx(struct os_mbuf *om)
{
    uint8_t ind = HCI_PKT_ACL;
    struct os_mbuf *x;
    int rc;

    rc = ipc_nrf5340_send(IPC_TX_CHANNEL, &ind, 1);
    if (rc == 0) {
        x = om;
        while (x) {
            rc = ipc_nrf5340_send(IPC_TX_CHANNEL, x->om_data, x->om_len);
            if (rc < 0) {
                break;
            }
            x = SLIST_NEXT(x, om_next);
        }
    }

    os_mbuf_free_chain(om);

    return (rc < 0) ? BLE_ERR_MEM_CAPACITY : 0;
}

#if MYNEWT_VAL(BLE_CONTROLLER)
void
ble_hci_trans_cfg_ll(ble_hci_trans_rx_cmd_fn *cmd_cb, void *cmd_arg,
                     ble_hci_trans_rx_acl_fn *acl_cb, void *acl_arg)
{
    nrf5340_ble_hci_api.cmd_cb = cmd_cb;
    nrf5340_ble_hci_api.cmd_arg = cmd_arg;
    nrf5340_ble_hci_api.acl_cb = acl_cb;
    nrf5340_ble_hci_api.acl_arg = acl_arg;
}

int
ble_hci_trans_ll_evt_tx(uint8_t *hci_ev)
{
    uint8_t ind = HCI_PKT_EVT;
    int len = 2 + hci_ev[1];
    int rc;

    rc = ipc_nrf5340_send(IPC_TX_CHANNEL, &ind, 1);
    if (rc == 0) {
        rc = ipc_nrf5340_send(IPC_TX_CHANNEL, hci_ev, len);
    }

    ble_hci_trans_buf_free(hci_ev);

    return (rc < 0) ? BLE_ERR_MEM_CAPACITY : 0;
}

int
ble_hci_trans_ll_acl_tx(struct os_mbuf *om)
{
    return ble_hci_trans_acl_tx(om);
}
#endif

#if MYNEWT_VAL(BLE_HOST)
void
ble_hci_trans_cfg_hs(ble_hci_trans_rx_cmd_fn *evt_cb, void *evt_arg,
                     ble_hci_trans_rx_acl_fn *acl_cb, void *acl_arg)
{
    nrf5340_ble_hci_api.evt_cb = evt_cb;
    nrf5340_ble_hci_api.evt_arg = evt_arg;
    nrf5340_ble_hci_api.acl_cb = acl_cb;
    nrf5340_ble_hci_api.acl_arg = acl_arg;
}

int
ble_hci_trans_hs_cmd_tx(uint8_t *cmd)
{
    uint8_t ind = HCI_PKT_CMD;
    int len = 3 + cmd[2];
    int rc;

    rc = ipc_nrf5340_send(IPC_TX_CHANNEL, &ind, 1);
    if (rc == 0) {
        rc = ipc_nrf5340_send(IPC_TX_CHANNEL, cmd, len);
    }

    ble_hci_trans_buf_free(cmd);

    return (rc < 0) ? BLE_ERR_MEM_CAPACITY :  0;
}

int
ble_hci_trans_hs_acl_tx(struct os_mbuf *om)
{
    return ble_hci_trans_acl_tx(om);
}
#endif

uint8_t *
ble_hci_trans_buf_alloc(int type)
{
    uint8_t *buf;

    switch (type) {
    case BLE_HCI_TRANS_BUF_CMD:
        assert(!nrf5340_ble_hci_pool_cmd.allocated);
        nrf5340_ble_hci_pool_cmd.allocated = 1;
        buf = nrf5340_ble_hci_pool_cmd.cmd;
        break;
    case BLE_HCI_TRANS_BUF_EVT_HI:
        buf = os_memblock_get(&nrf5340_ble_hci_pool_evt_hi);
        if (buf) {
            break;
        }
        /* no break */
    case BLE_HCI_TRANS_BUF_EVT_LO:
        buf = os_memblock_get(&nrf5340_ble_hci_pool_evt_lo);
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

    if (buf == nrf5340_ble_hci_pool_cmd.cmd) {
        assert(nrf5340_ble_hci_pool_cmd.allocated);
        nrf5340_ble_hci_pool_cmd.allocated = 0;
    } else if (os_memblock_from(&nrf5340_ble_hci_pool_evt_hi, buf)) {
        rc = os_memblock_put(&nrf5340_ble_hci_pool_evt_hi, buf);
        assert(rc == 0);
    } else {
        assert(os_memblock_from(&nrf5340_ble_hci_pool_evt_lo, buf));
        rc = os_memblock_put(&nrf5340_ble_hci_pool_evt_lo, buf);
        assert(rc == 0);
    }
}

static void
nrf5340_ble_hci_trans_rx_process(int channel)
{
    struct nrf5340_ble_hci_rx_data *rxd = &nrf5340_ble_hci_rx_data;
#if MYNEWT_VAL(BLE_HOST)
    int pool = BLE_HCI_TRANS_BUF_EVT_HI;
#endif
    int rc;

    switch (rxd->type) {
    case HCI_PKT_NONE:
        ipc_nrf5340_read(channel, &rxd->type, 1);
        rxd->len = 0;
        rxd->expected_len = 0;

#if MYNEWT_VAL(BLE_CONTROLLER)
        assert((rxd->type == HCI_PKT_ACL) || (rxd->type = HCI_PKT_CMD));
#endif
#if MYNEWT_VAL(BLE_HOST)
        assert((rxd->type == HCI_PKT_ACL) || (rxd->type = HCI_PKT_EVT));
#endif
        break;
#if MYNEWT_VAL(BLE_CONTROLLER)
    case HCI_PKT_CMD:
        /* commands are sent complete over IPC */
        rxd->len = ipc_nrf5340_read(channel, rxd->hdr, 3);
        assert(rxd->len == 3);

        rxd->buf = ble_hci_trans_buf_alloc(BLE_HCI_TRANS_BUF_CMD);
        memcpy(rxd->buf, rxd->hdr, rxd->len);

        rxd->len += ipc_nrf5340_read(channel, &rxd->buf[rxd->len], rxd->hdr[2]);
        assert(rxd->len == 3 + rxd->hdr[2]);

        rc = nrf5340_ble_hci_api.cmd_cb(rxd->buf, nrf5340_ble_hci_api.cmd_arg);
        if (rc != 0) {
            ble_hci_trans_buf_free(rxd->buf);
        }

        rxd->type = HCI_PKT_NONE;
        break;
#endif
#if MYNEWT_VAL(BLE_HOST)
    case HCI_PKT_EVT:
        /* events are sent complete over IPC */
        rxd->len = ipc_nrf5340_read(channel, rxd->hdr, 2);
        assert(rxd->len == 2);

        if (rxd->hdr[0] == BLE_HCI_EVCODE_LE_META) {
            /* For LE Meta event we need 3 bytes to parse header */
            rxd->len += ipc_nrf5340_read(channel, rxd->hdr + 2, 1);
            assert(rxd->len == 3);

            /* Advertising reports shall be allocated from low-prio pool */
            if ((rxd->hdr[2] == BLE_HCI_LE_SUBEV_ADV_RPT) ||
                (rxd->hdr[2] == BLE_HCI_LE_SUBEV_EXT_ADV_RPT)) {
                pool = BLE_HCI_TRANS_BUF_EVT_LO;
            }
        }

        rxd->buf = ble_hci_trans_buf_alloc(pool);
        if (!rxd->buf) {
            /*
             * Only care about valid buffer when shall be allocated from
             * high-prio pool, otherwise NULL is fine and we'll just skip
             * this event.
             */
            if (pool != BLE_HCI_TRANS_BUF_EVT_LO) {
                rxd->buf = ble_hci_trans_buf_alloc(BLE_HCI_TRANS_BUF_EVT_LO);
            }
        }

        rxd->expected_len = 2 + rxd->hdr[1];

        if (rxd->buf) {
            memcpy(rxd->buf, rxd->hdr, rxd->len);

            rxd->len += ipc_nrf5340_read(channel, &rxd->buf[rxd->len],
                                         rxd->expected_len - rxd->len);
            assert(rxd->expected_len == rxd->len);

            rc = nrf5340_ble_hci_api.evt_cb(rxd->buf,
                                            nrf5340_ble_hci_api.evt_arg);
            if (rc != 0) {
                ble_hci_trans_buf_free(rxd->buf);
            }
        } else {
            rxd->len += ipc_nrf5340_consume(channel,
                                            rxd->expected_len - rxd->len);
            assert(rxd->expected_len == rxd->len);
        }

        rxd->type = HCI_PKT_NONE;
        break;
#endif
    case HCI_PKT_ACL:
        if (rxd->len < 4) {
            rxd->len += ipc_nrf5340_read(channel, rxd->hdr, 4 - rxd->len);

            if (rxd->len < 4) {
                break;
            }
        }

        /* Parse header and allocate proper buffer if not done yet */
        if (rxd->expected_len == 0) {
            rxd->om = os_mbuf_get_pkthdr(&nrf5340_ble_hci_pool_acl_mbuf,
                                         sizeof(struct ble_mbuf_hdr));
            if (!rxd->om) {
                /* not much we can do here... */
                assert(0);
            }

            os_mbuf_append(rxd->om, rxd->hdr, rxd->len);
            rxd->expected_len = get_le16(&rxd->hdr[2]) + 4;
        }

        if (rxd->len != rxd->expected_len) {
            rxd->len += ipc_nrf5340_read_om(channel, rxd->om,
                                            rxd->expected_len - rxd->len);
        }

        if (rxd->len == rxd->expected_len) {
            rc = nrf5340_ble_hci_api.acl_cb(rxd->om,
                                            nrf5340_ble_hci_api.acl_arg);
            if (rc != 0) {
                os_mbuf_free_chain(rxd->om);
            }
            rxd->type = HCI_PKT_NONE;
        }
        break;
    default:
        assert(0);
        break;
    }
}

static void
nrf5340_ble_hci_trans_rx(int channel, void *user_data)
{
    while (ipc_nrf5340_available(channel) > 0) {
        nrf5340_ble_hci_trans_rx_process(channel);
    }
}

void
nrf5340_ble_hci_init(void)
{
    int rc;

    SYSINIT_ASSERT_ACTIVE();

    rc = os_mempool_init(&nrf5340_ble_hci_pool_acl, MYNEWT_VAL(BLE_ACL_BUF_COUNT),
                         POOL_ACL_BLOCK_SIZE, nrf5340_ble_hci_pool_acl_buf,
                         "nrf5340_ble_hci_pool_acl");
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = os_mbuf_pool_init(&nrf5340_ble_hci_pool_acl_mbuf,
                           &nrf5340_ble_hci_pool_acl, POOL_ACL_BLOCK_SIZE,
                           MYNEWT_VAL(BLE_ACL_BUF_COUNT));
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = os_mempool_init(&nrf5340_ble_hci_pool_evt_hi,
                         MYNEWT_VAL(BLE_HCI_EVT_HI_BUF_COUNT),
                         MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE),
                         nrf5340_ble_hci_pool_evt_hi_buf,
                         "nrf5340_ble_hci_pool_evt_hi");
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = os_mempool_init(&nrf5340_ble_hci_pool_evt_lo,
                         MYNEWT_VAL(BLE_HCI_EVT_LO_BUF_COUNT),
                         MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE),
                         nrf5340_ble_hci_pool_evt_lo_buf,
                         "nrf5340_ble_hci_pool_evt_lo");
    SYSINIT_PANIC_ASSERT(rc == 0);

    ipc_nrf5340_recv(IPC_RX_CHANNEL, nrf5340_ble_hci_trans_rx, NULL);
}
