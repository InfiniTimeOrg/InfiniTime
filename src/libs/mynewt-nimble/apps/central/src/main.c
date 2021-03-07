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

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "log/log.h"

static uint8_t g_own_addr_type;

static void
ble_app_set_addr(void)
{
    ble_addr_t addr;
    int rc;

    /* generate new non-resolvable private address */
    rc = ble_hs_id_gen_rnd(0, &addr);
    assert(rc == 0);

    /* set generated address */
    rc = ble_hs_id_set_rnd(addr.val);
    assert(rc == 0);
}

/* scan_event() calls scan(), so forward declaration is required */
static void scan(void);

/* connection has separate event handler from scan */
static int
conn_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            MODLOG_DFLT(INFO,"Connection was established\n");
            ble_gap_terminate(event->connect.conn_handle, 0x13);
        } else {
            MODLOG_DFLT(INFO,"Connection failed, error code: %i\n",
                event->connect.status);
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        MODLOG_DFLT(INFO,"Disconnected, reason code: %i\n",
        event->disconnect.reason);
        scan();
        break;
    case BLE_GAP_EVENT_CONN_UPDATE_REQ:
        MODLOG_DFLT(INFO,"Connection update request received\n");
        break;
    case BLE_GAP_EVENT_CONN_UPDATE:
        if (event->conn_update.status == 0) {
            MODLOG_DFLT(INFO,"Connection update successful\n");
        } else {
            MODLOG_DFLT(INFO,"Connection update failed; reson: %d\n",
                        event->conn_update.status);
        }
        break;
    default:
        MODLOG_DFLT(INFO,"Connection event type not supported, %d\n",
                    event->type);
        break;
    }
    return 0;
}

static int
scan_event(struct ble_gap_event *event, void *arg)
{
    /* predef_uuid stores information about UUID of device,
       that we connect to */
    const ble_uuid128_t predef_uuid =
        BLE_UUID128_INIT(0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                         0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff);
    struct ble_hs_adv_fields parsed_fields;
    int uuid_cmp_result;

    memset(&parsed_fields, 0, sizeof(parsed_fields));

    switch (event->type) {
    /* advertising report has been received during discovery procedure */
    case BLE_GAP_EVENT_DISC:
        MODLOG_DFLT(INFO, "Advertising report received! Checking UUID...\n");
        ble_hs_adv_parse_fields(&parsed_fields, event->disc.data,
                                event->disc.length_data);
        /* Predefined UUID is compared to recieved one;
           if doesn't fit - end procedure and go back to scanning,
           else - connect. */
        uuid_cmp_result = ble_uuid_cmp(&predef_uuid.u, &parsed_fields.uuids128->u);
        if (uuid_cmp_result) {
            MODLOG_DFLT(INFO, "UUID doesn't fit\n");
        } else {
            MODLOG_DFLT(INFO, "UUID fits, connecting...\n");
            ble_gap_disc_cancel();
            ble_gap_connect(g_own_addr_type, &(event->disc.addr), 10000,
                            NULL, conn_event, NULL);
        }
        break;
    case BLE_GAP_EVENT_DISC_COMPLETE:
        MODLOG_DFLT(INFO,"Discovery completed, reason: %d\n",
                    event->disc_complete.reason);
        scan();
        break;
    default:
        MODLOG_DFLT(ERROR, "Discovery event not handled\n");
        break;
    }
    return 0;
}

static void
scan(void)
{
    int rc;

    /* set scan parameters:
        - scan interval in 0.625ms units
        - scan window in 0.625ms units
        - filter policy - 0 if whitelisting not used
        - limited - should limited discovery be used
        - passive - should passive scan be used
        - filter duplicates - 1 enables filtering duplicated advertisements */
    const struct ble_gap_disc_params scan_params = {10000, 200, 0, 0, 0, 1};

    /* performs discovery procedure */
    rc = ble_gap_disc(g_own_addr_type, 10000, &scan_params,scan_event, NULL);
    assert(rc == 0);
}

static void
on_sync(void)
{
    int rc;
    /* Generate a non-resolvable private address. */
    ble_app_set_addr();

    /* g_own_addr_type will store type of addres our BSP uses */

    rc = ble_hs_util_ensure_addr(0);
    rc = ble_hs_id_infer_auto(0, &g_own_addr_type);
    assert(rc == 0);
    /* begin scanning */
    scan();
}

static void
on_reset(int reason)
{
    console_printf("Resetting state; reason=%d\n", reason);
}

int
main(int argc, char **argv)
{
    /* Initialize all packages. */
    sysinit();

    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.reset_cb = on_reset;

    /* As the last thing, process events from default event queue. */
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }

    return 0;
}
