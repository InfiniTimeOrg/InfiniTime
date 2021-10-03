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
#include "os/mynewt.h"
#include "nimble/ble.h"
#include "nimble/ble_hci_trans.h"
#include "nimble/hci_common.h"
#include "ble_hci_cmac_priv.h"

/*
 * If controller-to-host flow control is enabled we need to hold an extra command
 * buffer for HCI_Host_Number_Of_Completed_Packets which can be sent at any time.
 */
#if MYNEWT_VAL(BLE_HS_FLOW_CTRL) || MYNEWT_VAL(BLE_LL_CFG_FEAT_CTRL_TO_HOST_FLOW_CONTROL)
#define HCI_CMD_COUNT   2
#else
#define HCI_CMD_COUNT   1
#endif

#define POOL_ACL_BLOCK_SIZE     OS_ALIGN(MYNEWT_VAL(BLE_ACL_BUF_SIZE) +     \
                                         BLE_MBUF_MEMBLOCK_OVERHEAD +       \
                                         BLE_HCI_DATA_HDR_SZ, OS_ALIGNMENT)

static uint8_t ble_hci_pool_cmd_mempool_buf[
    OS_MEMPOOL_BYTES(HCI_CMD_COUNT, BLE_HCI_TRANS_CMD_SZ)];
static struct os_mempool ble_hci_pool_cmd_mempool;

static uint8_t ble_hci_pool_evt_hi_mempool_buf[
    OS_MEMPOOL_BYTES(MYNEWT_VAL(BLE_HCI_EVT_HI_BUF_COUNT),
                     MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE))];
static struct os_mempool ble_hci_pool_evt_hi_mempool;

static uint8_t ble_hci_pool_evt_lo_mempool_buf[
    OS_MEMPOOL_BYTES(MYNEWT_VAL(BLE_HCI_EVT_LO_BUF_COUNT),
                     MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE))];
static struct os_mempool ble_hci_pool_evt_lo_mempool;

static uint8_t ble_hci_pool_acl_mempool_buf[
    OS_MEMPOOL_BYTES(MYNEWT_VAL(BLE_ACL_BUF_COUNT),
                     POOL_ACL_BLOCK_SIZE)];
static struct os_mempool_ext ble_hci_pool_acl_mempool;
static struct os_mbuf_pool ble_hci_pool_acl_mbuf_pool;

__attribute__((weak)) void ble_hci_trans_notify_free(void);

static os_mempool_put_fn *g_ble_hci_pool_acl_mempool_put_cb;
static void *g_ble_hci_pool_acl_mempool_put_arg;

int
ble_hci_trans_reset(void)
{
    return 0;
}

uint8_t *
ble_hci_trans_buf_alloc(int type)
{
    uint8_t *buf;

    switch (type) {
    case BLE_HCI_TRANS_BUF_CMD:
        buf = os_memblock_get(&ble_hci_pool_cmd_mempool);
        break;
    case BLE_HCI_TRANS_BUF_EVT_HI:
        buf = os_memblock_get(&ble_hci_pool_evt_hi_mempool);
        if (buf) {
            break;
        }
    /* no break */
    case BLE_HCI_TRANS_BUF_EVT_LO:
        buf = os_memblock_get(&ble_hci_pool_evt_lo_mempool);
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

    if (os_memblock_from(&ble_hci_pool_cmd_mempool, buf)) {
        rc = os_memblock_put(&ble_hci_pool_cmd_mempool, buf);
        assert(rc == 0);
    } else if (os_memblock_from(&ble_hci_pool_evt_hi_mempool, buf)) {
        rc = os_memblock_put(&ble_hci_pool_evt_hi_mempool, buf);
        assert(rc == 0);
    } else {
        assert(os_memblock_from(&ble_hci_pool_evt_lo_mempool, buf));
        rc = os_memblock_put(&ble_hci_pool_evt_lo_mempool, buf);
        assert(rc == 0);
    }

    ble_hci_trans_notify_free();
}

struct os_mbuf *
ble_hci_cmac_alloc_acl_mbuf(void)
{
    return os_mbuf_get_pkthdr(&ble_hci_pool_acl_mbuf_pool,
                              sizeof(struct ble_mbuf_hdr));
}

static os_error_t
ble_hci_cmac_free_acl_cb(struct os_mempool_ext *mpe, void *data, void *arg)
{
    int rc;

    if (g_ble_hci_pool_acl_mempool_put_cb) {
        rc = g_ble_hci_pool_acl_mempool_put_cb(mpe, data,
                                               g_ble_hci_pool_acl_mempool_put_arg);
    } else {
        rc = os_memblock_put_from_cb(&mpe->mpe_mp, data);
    }

    if (rc != 0) {
        return rc;
    }

    ble_hci_trans_notify_free();

    return 0;
}


int
ble_hci_trans_set_acl_free_cb(os_mempool_put_fn *cb, void *arg)
{
    g_ble_hci_pool_acl_mempool_put_cb = cb;
    g_ble_hci_pool_acl_mempool_put_arg = arg;

    return 0;
}

void
ble_hci_cmac_init(void)
{
    int rc;

    SYSINIT_ASSERT_ACTIVE();

    rc = os_mempool_init(&ble_hci_pool_cmd_mempool,
                         HCI_CMD_COUNT, BLE_HCI_TRANS_CMD_SZ,
                         ble_hci_pool_cmd_mempool_buf, "ble_hci_cmd");
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = os_mempool_init(&ble_hci_pool_evt_hi_mempool,
                         MYNEWT_VAL(BLE_HCI_EVT_HI_BUF_COUNT),
                         MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE),
                         ble_hci_pool_evt_hi_mempool_buf, "ble_hci_evt_hi");
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = os_mempool_init(&ble_hci_pool_evt_lo_mempool,
                         MYNEWT_VAL(BLE_HCI_EVT_LO_BUF_COUNT),
                         MYNEWT_VAL(BLE_HCI_EVT_BUF_SIZE),
                         ble_hci_pool_evt_lo_mempool_buf, "ble_hci_evt_lo");
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = os_mempool_ext_init(&ble_hci_pool_acl_mempool,
                             MYNEWT_VAL(BLE_ACL_BUF_COUNT), POOL_ACL_BLOCK_SIZE,
                             ble_hci_pool_acl_mempool_buf, "ble_hci_acl");
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = os_mbuf_pool_init(&ble_hci_pool_acl_mbuf_pool,
                           &ble_hci_pool_acl_mempool.mpe_mp, POOL_ACL_BLOCK_SIZE,
                           MYNEWT_VAL(BLE_ACL_BUF_COUNT));
    SYSINIT_PANIC_ASSERT(rc == 0);

    ble_hci_pool_acl_mempool.mpe_put_cb = ble_hci_cmac_free_acl_cb;
}
