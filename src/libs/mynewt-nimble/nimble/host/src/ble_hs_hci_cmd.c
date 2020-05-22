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
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "os/os.h"
#include "nimble/hci_common.h"
#include "nimble/ble_hci_trans.h"
#include "host/ble_monitor.h"
#include "ble_hs_priv.h"
#include "ble_monitor_priv.h"

static int
ble_hs_hci_cmd_transport(struct ble_hci_cmd *cmd)
{
    int rc;

#if BLE_MONITOR
    ble_monitor_send(BLE_MONITOR_OPCODE_COMMAND_PKT, cmd,
                     cmd->length + sizeof(*cmd));
#endif

    rc = ble_hci_trans_hs_cmd_tx((uint8_t *) cmd);
    switch (rc) {
    case 0:
        return 0;

    case BLE_ERR_MEM_CAPACITY:
        return BLE_HS_ENOMEM_EVT;

    default:
        return BLE_HS_EUNKNOWN;
    }
}

static int
ble_hs_hci_cmd_send(uint16_t opcode, uint8_t len, const void *cmddata)
{
    struct ble_hci_cmd *cmd;
    int rc;

    cmd = (void *) ble_hci_trans_buf_alloc(BLE_HCI_TRANS_BUF_CMD);
    BLE_HS_DBG_ASSERT(cmd != NULL);

    cmd->opcode = htole16(opcode);
    cmd->length = len;
    if (len != 0) {
        memcpy(cmd->data, cmddata, len);
    }

    rc = ble_hs_hci_cmd_transport(cmd);

    if (rc == 0) {
        STATS_INC(ble_hs_stats, hci_cmd);
    } else {
        BLE_HS_LOG(DEBUG, "ble_hs_hci_cmd_send failure; rc=%d\n", rc);
    }

    return rc;
}

int
ble_hs_hci_cmd_send_buf(uint16_t opcode, const void *buf, uint8_t buf_len)
{
    switch (ble_hs_sync_state) {
    case BLE_HS_SYNC_STATE_BAD:
        return BLE_HS_ENOTSYNCED;

    case BLE_HS_SYNC_STATE_BRINGUP:
        if (!ble_hs_is_parent_task()) {
            return BLE_HS_ENOTSYNCED;
        }
        break;

    case BLE_HS_SYNC_STATE_GOOD:
        break;

    default:
        BLE_HS_DBG_ASSERT(0);
        return BLE_HS_EUNKNOWN;
    }

    return ble_hs_hci_cmd_send(opcode, buf_len, buf);
}
