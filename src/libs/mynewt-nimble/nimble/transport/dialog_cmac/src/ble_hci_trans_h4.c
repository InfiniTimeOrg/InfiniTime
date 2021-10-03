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
#include <stdbool.h>
#include <string.h>
#include "syscfg/syscfg.h"
#include "os/os_mbuf.h"
#include "nimble/ble_hci_trans.h"
#include "nimble/hci_common.h"
#include "ble_hci_trans_h4.h"
#include "ble_hci_cmac_priv.h"

#define RXS_STATE_W4_PKT_TYPE       0
#define RXS_STATE_W4_HEADER         1
#define RXS_STATE_W4_PAYLOAD        2
#define RXS_STATE_COMPLETED         3

struct input_buffer {
    const uint8_t *buf;
    uint16_t len;
};

static int
ble_hci_trans_h4_ib_adjust(struct input_buffer *ib, uint16_t len)
{
    assert(ib->len >= len);

    ib->buf += len;
    ib->len -= len;

    return len;
}

static void
ble_hci_trans_h4_rxs_start(struct ble_hci_trans_h4_rx_state *rxs, uint8_t pkt_type)
{
    rxs->pkt_type = pkt_type;
    rxs->len = 0;
    rxs->expected_len = 0;

    switch (rxs->pkt_type) {
#if MYNEWT_VAL(BLE_CONTROLLER)
    case BLE_HCI_TRANS_H4_PKT_TYPE_CMD:
        rxs->min_len = 3;
        break;
#endif
    case BLE_HCI_TRANS_H4_PKT_TYPE_ACL:
        rxs->min_len = 4;
        break;
#if MYNEWT_VAL(BLE_HOST)
    case BLE_HCI_TRANS_H4_PKT_TYPE_EVT:
        rxs->min_len = 2;
        break;
#endif
    default:
        /* XXX sync loss */
        assert(0);
        break;
    }
}

static int
ble_hci_trans_h4_pull_min_len(struct ble_hci_trans_h4_rx_state *rxs,
                              struct input_buffer *ib)
{
    uint16_t len;

    len = min(ib->len, rxs->min_len - rxs->len);
    memcpy(&rxs->hdr[rxs->len], ib->buf, len);

    rxs->len += len;
    ble_hci_trans_h4_ib_adjust(ib, len);

    return rxs->len != rxs->min_len;
}

static int
ble_hci_trans_h4_rx_state_w4_header(struct ble_hci_trans_h4_rx_state *rxs,
                                    struct input_buffer *ib)
{
#if MYNEWT_VAL(BLE_HOST)
    int pool;
#endif
    int rc;

    rc = ble_hci_trans_h4_pull_min_len(rxs, ib);
    if (rc) {
        /* need more data */
        return 1;
    }

    switch (rxs->pkt_type) {
#if MYNEWT_VAL(BLE_CONTROLLER)
    case BLE_HCI_TRANS_H4_PKT_TYPE_CMD:
        rxs->buf = ble_hci_trans_buf_alloc(BLE_HCI_TRANS_BUF_CMD);
        if (!rxs->buf) {
            return -1;
        }

        memcpy(rxs->buf, rxs->hdr, rxs->len);
        rxs->expected_len = rxs->hdr[2] + 3;
        break;
#endif
    case BLE_HCI_TRANS_H4_PKT_TYPE_ACL:
        rxs->om = ble_hci_cmac_alloc_acl_mbuf();
        if (!rxs->om) {
            return -1;
        }

        os_mbuf_append(rxs->om, rxs->hdr, rxs->len);
        rxs->expected_len = get_le16(&rxs->hdr[2]) + 4;
        break;
#if MYNEWT_VAL(BLE_HOST)
    case BLE_HCI_TRANS_H4_PKT_TYPE_EVT:
        pool = BLE_HCI_TRANS_BUF_EVT_HI;
        if (rxs->hdr[0] == BLE_HCI_EVCODE_LE_META) {
            /* For LE Meta event we need 3 bytes to parse header */
            rxs->min_len = 3;
            rc = ble_hci_trans_h4_pull_min_len(rxs, ib);
            if (rc) {
                /* need more data */
                return 1;
            }

            /* Advertising reports shall be allocated from low-prio pool */
            if ((rxs->hdr[2] == BLE_HCI_LE_SUBEV_ADV_RPT) ||
                (rxs->hdr[2] == BLE_HCI_LE_SUBEV_EXT_ADV_RPT)) {
                pool = BLE_HCI_TRANS_BUF_EVT_LO;
            }
        }

        /*
         * XXX Events originally allocated from hi-pool can use lo-pool as
         *     fallback and cannot be dropped. Events allocated from lo-pool
         *     can be dropped to avoid oom while scanning which means that
         *     any advertising or extended advertising report can be silently
         *     discarded by transport. While this is perfectly fine for legacy
         *     advertising, for extended advertising it means we can drop start
         *     or end of chain report and host won't be able to reassemble
         *     chain properly... so just need to make sure pool on host side is
         *     large enough to catch up with controller.
         */
        rxs->buf = ble_hci_trans_buf_alloc(pool);
        if (!rxs->buf && pool == BLE_HCI_TRANS_BUF_EVT_HI) {
            rxs->buf = ble_hci_trans_buf_alloc(BLE_HCI_TRANS_BUF_EVT_LO);
            if (!rxs->buf) {
                return -1;
            }
        }

        if (rxs->buf) {
            memcpy(rxs->buf, rxs->hdr, rxs->len);
        }

        rxs->expected_len = rxs->hdr[1] + 2;
        break;
#endif
    default:
        assert(0);
        break;
    }

    return 0;
}

static int
ble_hci_trans_h4_rx_state_w4_payload(struct ble_hci_trans_h4_rx_state *rxs,
                                     struct input_buffer *ib)
{
    uint16_t mbuf_len;
    uint16_t len;
    int rc;

    len = min(ib->len, rxs->expected_len - rxs->len);

    switch (rxs->pkt_type) {
#if MYNEWT_VAL(BLE_CONTROLLER)
    case BLE_HCI_TRANS_H4_PKT_TYPE_CMD:
#endif
#if MYNEWT_VAL(BLE_HOST)
    case BLE_HCI_TRANS_H4_PKT_TYPE_EVT:
#endif
        if (rxs->buf) {
            memcpy(&rxs->buf[rxs->len], ib->buf, len);
        }
        break;
    case BLE_HCI_TRANS_H4_PKT_TYPE_ACL:
        assert(rxs->om);

        mbuf_len = OS_MBUF_PKTLEN(rxs->om);
        rc = os_mbuf_append(rxs->om, ib->buf, len);
        if (rc) {
            /*
             * Some data may already be appended so need to adjust rxs only by
             * the size of appended data.
             */
            len = OS_MBUF_PKTLEN(rxs->om) - mbuf_len;
            rxs->len += len;
            ble_hci_trans_h4_ib_adjust(ib, len);

            return -1;
        }
        break;
    default:
        assert(0);
        break;
    }

    rxs->len += len;
    ble_hci_trans_h4_ib_adjust(ib, len);

    /* return 1 if need more data */
    return rxs->len != rxs->expected_len;
}

static void
ble_hci_trans_h4_rx_state_completed(struct ble_hci_trans_h4_rx_state *rxs,
                                    ble_hci_trans_h4_frame_cb *frame_cb)
{
    int rc;

    switch (rxs->pkt_type) {
#if MYNEWT_VAL(BLE_CONTROLLER)
    case BLE_HCI_TRANS_H4_PKT_TYPE_CMD:
#endif
#if MYNEWT_VAL(BLE_HOST)
    case BLE_HCI_TRANS_H4_PKT_TYPE_EVT:
#endif
        if (rxs->buf) {
            rc = frame_cb(rxs->pkt_type, rxs->buf);
            if (rc != 0) {
                ble_hci_trans_buf_free(rxs->buf);
            }
            rxs->buf = NULL;
        }
        break;
    case BLE_HCI_TRANS_H4_PKT_TYPE_ACL:
        if (rxs->om) {
            rc = frame_cb(rxs->pkt_type, rxs->om);
            if (rc != 0) {
                os_mbuf_free_chain(rxs->om);
            }
            rxs->om = NULL;
        }
        break;
    default:
        assert(0);
        break;
    }
}

int
ble_hci_trans_h4_rx(struct ble_hci_trans_h4_rx_state *rxs, const uint8_t *buf,
                    uint16_t len, ble_hci_trans_h4_frame_cb *frame_cb)
{
    struct input_buffer ib = {
        .buf = buf,
        .len = len,
    };
    int rc = 0;

    while (ib.len && (rc >= 0)) {
        rc = 0;
        switch (rxs->state) {
        case RXS_STATE_W4_PKT_TYPE:
            ble_hci_trans_h4_rxs_start(rxs, ib.buf[0]);
            ble_hci_trans_h4_ib_adjust(&ib, 1);
            rxs->state = RXS_STATE_W4_HEADER;
        /* no break */

        case RXS_STATE_W4_HEADER:
            rc = ble_hci_trans_h4_rx_state_w4_header(rxs, &ib);
            if (rc) {
                break;
            }
            rxs->state = RXS_STATE_W4_PAYLOAD;
        /* no break */

        case RXS_STATE_W4_PAYLOAD:
            rc = ble_hci_trans_h4_rx_state_w4_payload(rxs, &ib);
            if (rc) {
                break;
            }
            rxs->state = RXS_STATE_COMPLETED;
        /* no break */

        case RXS_STATE_COMPLETED:
            ble_hci_trans_h4_rx_state_completed(rxs, frame_cb);
            rxs->state = RXS_STATE_W4_PKT_TYPE;
            break;

        default:
            assert(0);
            /* consume all remaining data */
            ble_hci_trans_h4_ib_adjust(&ib, ib.len);
            break;
        }
    }

    /*
     * Calculate consumed bytes
     *
     * Note: we should always consume some bytes unless there is an oom error.
     * It's also possible that we have an oom error but already consumed some
     * data, in such case just return success and error will be returned on next
     * pass.
     */
    len = len - ib.len;
    if (len == 0) {
        assert(rc < 0);
        return -1;
    }

    return len;
}
