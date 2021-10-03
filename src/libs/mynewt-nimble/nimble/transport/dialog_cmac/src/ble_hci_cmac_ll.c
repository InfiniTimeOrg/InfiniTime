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

#include "syscfg/syscfg.h"

#if MYNEWT_VAL(BLE_CONTROLLER)

#if !MYNEWT_VAL(MCU_DEBUG_DSER_BLE_HCI_CMAC_LL)
#define MCU_DIAG_SER_DISABLE
#endif

#include <assert.h>
#include <string.h>
#include "mcu/mcu.h"
#include "cmac_driver/cmac_shared.h"
#include "nimble/ble_hci_trans.h"
#include "os/os_mbuf.h"
#include "ble_hci_trans_h4.h"
#include "ble_hci_cmac_priv.h"

struct ble_hci_cmac_ll_api {
    ble_hci_trans_rx_cmd_fn *cmd_cb;
    void *cmd_arg;
    ble_hci_trans_rx_acl_fn *acl_cb;
    void *acl_arg;
};

static struct ble_hci_cmac_ll_api g_ble_hci_cmac_ll_api;
static struct ble_hci_trans_h4_rx_state g_ble_hci_cmac_ll_rx_state;

static int
ble_hci_cmac_ll_frame_cb(uint8_t pkt_type, void *data)
{
    int rc;

    MCU_DIAG_SER('F');

    switch (pkt_type) {
    case BLE_HCI_TRANS_H4_PKT_TYPE_CMD:
        rc = g_ble_hci_cmac_ll_api.cmd_cb(data, g_ble_hci_cmac_ll_api.cmd_arg);
        break;
    case BLE_HCI_TRANS_H4_PKT_TYPE_ACL:
        rc = g_ble_hci_cmac_ll_api.acl_cb(data, g_ble_hci_cmac_ll_api.acl_arg);
        break;
    default:
        assert(0);
        break;
    }

    return rc;
}

static int
ble_hci_cmac_ll_mbox_read_cb(const void *data, uint16_t len)
{
    int rlen;

    MCU_DIAG_SER('R');
    rlen = ble_hci_trans_h4_rx(&g_ble_hci_cmac_ll_rx_state, data, len,
                               ble_hci_cmac_ll_frame_cb);

    /* There should be no oom on LL side due to flow control used */
    assert(rlen >= 0);

    return rlen;
}

static void
ble_hci_cmac_ll_mbox_write_notif_cb(void)
{
    MCU_DIAG_SER('W');
    CMAC->CM_EV_SET_REG = CMAC_CM_EV_SET_REG_EV1C_CMAC2SYS_IRQ_SET_Msk;
}

int
ble_hci_trans_ll_evt_tx(uint8_t *evt)
{
    uint8_t pkt_type = BLE_HCI_TRANS_H4_PKT_TYPE_EVT;

    cmac_mbox_write(&pkt_type, sizeof(pkt_type));
    cmac_mbox_write(evt, evt[1] + 2);

    ble_hci_trans_buf_free(evt);

    return 0;
}

int
ble_hci_trans_ll_acl_tx(struct os_mbuf *om)
{
    uint8_t pkt_type = BLE_HCI_TRANS_H4_PKT_TYPE_ACL;
    struct os_mbuf *om_next;

    cmac_mbox_write(&pkt_type, sizeof(pkt_type));

    while (om) {
        om_next = SLIST_NEXT(om, om_next);

        cmac_mbox_write(om->om_data, om->om_len);

        os_mbuf_free(om);
        om = om_next;
    }

    return 0;
}

void
ble_hci_trans_cfg_ll(ble_hci_trans_rx_cmd_fn *cmd_cb, void *cmd_arg,
                     ble_hci_trans_rx_acl_fn *acl_cb, void *acl_arg)
{
    g_ble_hci_cmac_ll_api.cmd_cb = cmd_cb;
    g_ble_hci_cmac_ll_api.cmd_arg = cmd_arg;
    g_ble_hci_cmac_ll_api.acl_cb = acl_cb;
    g_ble_hci_cmac_ll_api.acl_arg = acl_arg;

    /* Setup callbacks for mailboxes */
    cmac_mbox_set_read_cb(ble_hci_cmac_ll_mbox_read_cb);
    cmac_mbox_set_write_notif_cb(ble_hci_cmac_ll_mbox_write_notif_cb);

    /* Synchronize with SYS */
    cmac_shared_sync();
}

#endif
