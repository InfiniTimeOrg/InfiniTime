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

/* scan_event() calls scan(), so forward declaration is required */
static void scan(void);

static void
ble_app_set_addr(void)
{
    ble_addr_t addr;
    int rc;

    /* generate new non-resolvable private address */
    rc = ble_hs_id_gen_rnd(1, &addr);
    assert(rc == 0);

    /* set generated address */
    rc = ble_hs_id_set_rnd(addr.val);
    assert(rc == 0);
}

static void
print_uuid(const ble_uuid_t *uuid)
{
    char buf[BLE_UUID_STR_LEN];

    MODLOG_DFLT(DEBUG, "%s", ble_uuid_to_str(uuid, buf));
}

/* Utility function to log an array of bytes. */
static void
print_bytes(const uint8_t *bytes, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        MODLOG_DFLT(DEBUG, "%s0x%02x", i != 0 ? ":" : "", bytes[i]);
    }
}

static char *
addr_str(const void *addr)
{
    static char buf[6 * 2 + 5 + 1];
    const uint8_t *u8p;

    u8p = addr;
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
            u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);

    return buf;
}

static void
print_adv_fields(const struct ble_hs_adv_fields *fields)
{
    char s[BLE_HS_ADV_MAX_SZ];
    const uint8_t *u8p;
    int i;

    if (fields->flags != 0) {
        MODLOG_DFLT(DEBUG, "    flags=0x%02x\n", fields->flags);
    }

    if (fields->uuids16 != NULL) {
        MODLOG_DFLT(DEBUG, "    uuids16(%scomplete)=",
                    fields->uuids16_is_complete ? "" : "in");
        for (i = 0; i < fields->num_uuids16; i++) {
            print_uuid(&fields->uuids16[i].u);
            MODLOG_DFLT(DEBUG, " ");
        }
        MODLOG_DFLT(DEBUG, "\n");
    }

    if (fields->uuids32 != NULL) {
        MODLOG_DFLT(DEBUG, "    uuids32(%scomplete)=",
                    fields->uuids32_is_complete ? "" : "in");
        for (i = 0; i < fields->num_uuids32; i++) {
            print_uuid(&fields->uuids32[i].u);
            MODLOG_DFLT(DEBUG, " ");
        }
        MODLOG_DFLT(DEBUG, "\n");
    }

    if (fields->uuids128 != NULL) {
        MODLOG_DFLT(DEBUG, "    uuids128(%scomplete)=",
                    fields->uuids128_is_complete ? "" : "in");
        for (i = 0; i < fields->num_uuids128; i++) {
            print_uuid(&fields->uuids128[i].u);
            MODLOG_DFLT(DEBUG, " ");
        }
        MODLOG_DFLT(DEBUG, "\n");
    }

    if (fields->name != NULL) {
        assert(fields->name_len < sizeof s - 1);
        memcpy(s, fields->name, fields->name_len);
        s[fields->name_len] = '\0';
        MODLOG_DFLT(DEBUG, "    name(%scomplete)=%s\n",
                    fields->name_is_complete ? "" : "in", s);
    }

    if (fields->tx_pwr_lvl_is_present) {
        MODLOG_DFLT(DEBUG, "    tx_pwr_lvl=%d\n", fields->tx_pwr_lvl);
    }

    if (fields->slave_itvl_range != NULL) {
        MODLOG_DFLT(DEBUG, "    slave_itvl_range=");
        print_bytes(fields->slave_itvl_range, BLE_HS_ADV_SLAVE_ITVL_RANGE_LEN);
        MODLOG_DFLT(DEBUG, "\n");
    }

    if (fields->svc_data_uuid16 != NULL) {
        MODLOG_DFLT(DEBUG, "    svc_data_uuid16=");
        print_bytes(fields->svc_data_uuid16, fields->svc_data_uuid16_len);
        MODLOG_DFLT(DEBUG, "\n");
    }

    if (fields->public_tgt_addr != NULL) {
        MODLOG_DFLT(DEBUG, "    public_tgt_addr=");
        u8p = fields->public_tgt_addr;
        for (i = 0; i < fields->num_public_tgt_addrs; i++) {
            MODLOG_DFLT(DEBUG, "public_tgt_addr=%s ", addr_str(u8p));
            u8p += BLE_HS_ADV_PUBLIC_TGT_ADDR_ENTRY_LEN;
        }
        MODLOG_DFLT(DEBUG, "\n");
    }

    if (fields->appearance_is_present) {
        MODLOG_DFLT(DEBUG, "    appearance=0x%04x\n", fields->appearance);
    }

    if (fields->adv_itvl_is_present) {
        MODLOG_DFLT(DEBUG, "    adv_itvl=0x%04x\n", fields->adv_itvl);
    }

    if (fields->svc_data_uuid32 != NULL) {
        MODLOG_DFLT(DEBUG, "    svc_data_uuid32=");
        print_bytes(fields->svc_data_uuid32, fields->svc_data_uuid32_len);
        MODLOG_DFLT(DEBUG, "\n");
    }

    if (fields->svc_data_uuid128 != NULL) {
        MODLOG_DFLT(DEBUG, "    svc_data_uuid128=");
        print_bytes(fields->svc_data_uuid128, fields->svc_data_uuid128_len);
        MODLOG_DFLT(DEBUG, "\n");
    }

    if (fields->uri != NULL) {
        MODLOG_DFLT(DEBUG, "    uri=");
        print_bytes(fields->uri, fields->uri_len);
        MODLOG_DFLT(DEBUG, "\n");
    }

    if (fields->mfg_data != NULL) {
        MODLOG_DFLT(DEBUG, "    mfg_data=");
        print_bytes(fields->mfg_data, fields->mfg_data_len);
        MODLOG_DFLT(DEBUG, "\n");
    }
}

static int
scan_event(struct ble_gap_event *event, void *arg)
{
    struct ble_hs_adv_fields fields;
    int rc;
    switch (event->type) {
    /* advertising report has been received during discovery procedure */
    case BLE_GAP_EVENT_DISC:
        MODLOG_DFLT(ERROR, "Advertising report received!\n");
        rc = ble_hs_adv_parse_fields(&fields, event->disc.data,
                                     event->disc.length_data);
        if (rc != 0) {
            return 0;
        }
        print_adv_fields(&fields);
        return 0;
    /* discovery procedure has terminated */
    case BLE_GAP_EVENT_DISC_COMPLETE:
        MODLOG_DFLT(INFO, "Discovery completed, terminaton code: %d\n",
                    event->disc_complete.reason);
        scan();
        return 0;
    default:
        MODLOG_DFLT(ERROR, "Discovery event not handled\n");
        return 0;
    }
}

static void
scan(void)
{
    /* set scan parameters */
    struct ble_gap_disc_params scan_params;
    scan_params.itvl = 500;
    scan_params.window = 250;
    scan_params.filter_policy = 0;
    scan_params.limited = 0;
    scan_params.passive = 1;
    scan_params.filter_duplicates = 1;
    /* performs discovery procedure; value of own_addr_type is hard-coded,
       because NRPA is used */
    ble_gap_disc(BLE_OWN_ADDR_RANDOM, 1000, &scan_params, scan_event, NULL);
}

static void
on_sync(void)
{
    /* Generate a non-resolvable private address. */
    ble_app_set_addr();

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
