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
#include "syscfg/syscfg.h"

#if MYNEWT_VAL(BLE_HOST)

#include "cmac_driver/cmac_shared.h"
#include "cmac_driver/cmac_host.h"
#include "nimble/ble_hci_trans.h"
#include "os/os_mbuf.h"
#include "ble_hci_trans_h4.h"
#include "ble_hci_cmac_priv.h"

struct ble_hci_cmac_hs_api {
    ble_hci_trans_rx_cmd_fn *evt_cb;
    void *evt_arg;
    ble_hci_trans_rx_acl_fn *acl_cb;
    void *acl_arg;
};

static struct ble_hci_cmac_hs_api g_ble_hci_cmac_hs_api;
static struct ble_hci_trans_h4_rx_state g_ble_hci_cmac_hs_rx_state;
static bool g_ble_hci_cmac_hs_read_err;

static int
ble_hci_cmac_hs_frame_cb(uint8_t pkt_type, void *data)
{
    int rc;

    switch (pkt_type) {
    case BLE_HCI_TRANS_H4_PKT_TYPE_ACL:
        rc = g_ble_hci_cmac_hs_api.acl_cb(data, g_ble_hci_cmac_hs_api.acl_arg);
        break;
    case BLE_HCI_TRANS_H4_PKT_TYPE_EVT:
        rc = g_ble_hci_cmac_hs_api.evt_cb(data, g_ble_hci_cmac_hs_api.evt_arg);
        break;
    default:
        assert(0);
        break;
    }

    return rc;
}

static int
ble_hci_cmac_hs_mbox_read_cb(const void *data, uint16_t len)
{
    int rlen;
    os_sr_t sr;

    rlen = ble_hci_trans_h4_rx(&g_ble_hci_cmac_hs_rx_state, data, len,
                               ble_hci_cmac_hs_frame_cb);
    if (rlen < 0) {
        /*
         * There was oom error, we need to wait for buffer to be freed and
         * trigger another read.
         */
        OS_ENTER_CRITICAL(sr);
        g_ble_hci_cmac_hs_read_err = true;
        OS_EXIT_CRITICAL(sr);
    }

    return rlen;
}

static void
ble_hci_cmac_hs_mbox_write_notif_cb(void)
{
    cmac_host_signal2cmac();
}

int
ble_hci_trans_hs_cmd_tx(uint8_t *cmd)
{
    uint8_t pkt_type = BLE_HCI_TRANS_H4_PKT_TYPE_CMD;

    cmac_mbox_write(&pkt_type, sizeof(pkt_type));
    cmac_mbox_write(cmd, cmd[2] + 3);

    ble_hci_trans_buf_free(cmd);

    return 0;
}

int
ble_hci_trans_hs_acl_tx(struct os_mbuf *om)
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
ble_hci_trans_notify_free(void)
{
    os_sr_t sr;

    OS_ENTER_CRITICAL(sr);
    if (g_ble_hci_cmac_hs_read_err) {
        g_ble_hci_cmac_hs_read_err = false;
        /* Just trigger an interrupt, it will trigger read */
        NVIC_SetPendingIRQ(CMAC2SYS_IRQn);
    }
    OS_EXIT_CRITICAL(sr);
}

void
ble_hci_trans_cfg_hs(ble_hci_trans_rx_cmd_fn *evt_cb, void *evt_arg,
                     ble_hci_trans_rx_acl_fn *acl_cb, void *acl_arg)
{
    g_ble_hci_cmac_hs_api.evt_cb = evt_cb;
    g_ble_hci_cmac_hs_api.evt_arg = evt_arg;
    g_ble_hci_cmac_hs_api.acl_cb = acl_cb;
    g_ble_hci_cmac_hs_api.acl_arg = acl_arg;

    /* We can now handle data from CMAC, initialize it */
    cmac_mbox_set_read_cb(ble_hci_cmac_hs_mbox_read_cb);
    cmac_mbox_set_write_notif_cb(ble_hci_cmac_hs_mbox_write_notif_cb);
    cmac_host_init();
}

#endif
