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
#include <stdio.h>
#include <string.h>

#include "services/gap/ble_svc_gap.h"
#include "bsp/bsp.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"

#include "mesh.h"
#include "board.h"
#include "mesh_badge.h"

static const ble_uuid16_t gatt_cud_uuid = BLE_UUID16_INIT(0x2901);
static const ble_uuid16_t gatt_cpf_uuid = BLE_UUID16_INIT(0x2904);

/** @brief GATT Characteristic Presentation Format Attribute Value. */
struct bt_gatt_cpf {
    /** Format of the value of the characteristic */
    uint8_t format;
    /** Exponent field to determine how the value of this characteristic is further formatted */
    int8_t exponent;
    /** Unit of the characteristic */
    uint16_t unit;
    /** Name space of the description */
    uint8_t name_space;
    /** Description of the characteristic as defined in a higher layer profile */
    uint16_t description;
} __packed;

#define CPF_FORMAT_UTF8 0x19

static const struct bt_gatt_cpf name_cpf = {
        .format = CPF_FORMAT_UTF8,
};

static const ble_uuid128_t name_uuid = BLE_UUID128_INIT(
        0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
        0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

static const ble_uuid128_t name_enc_uuid = BLE_UUID128_INIT(
        0xf1, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
        0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

static int
gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                    struct ble_gatt_access_ctxt *ctxt,
                    void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &name_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) { {
            .uuid = &name_enc_uuid.u,
            .access_cb = gatt_svr_chr_access,
            .flags = BLE_GATT_CHR_F_READ |
                    BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            .descriptors = (struct ble_gatt_dsc_def[]) { {
                    .uuid = &gatt_cud_uuid.u,
                    .access_cb = gatt_svr_chr_access,
                    .att_flags = BLE_ATT_F_READ,
                }, {
                    .uuid = &gatt_cpf_uuid.u,
                    .access_cb = gatt_svr_chr_access,
                    .att_flags = BLE_ATT_F_READ,
                }, {
                    0, /* No more descriptors in this characteristic. */
                } }
        }, {
            0, /* No more characteristics in this service. */
        } },
    },

    {
        0, /* No more services. */
    },
};

static int read_name(struct os_mbuf *om)
{
    const char *value = bt_get_name();
    int rc;

    rc = os_mbuf_append(om, value, (uint16_t) strlen(value));
    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int write_name(struct os_mbuf *om)
{
    char name[MYNEWT_VAL(BLE_SVC_GAP_DEVICE_NAME_MAX_LENGTH)];
    uint16_t len;
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len >= sizeof(name)) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, name, sizeof(name) - 1, &len);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    name[len] = '\0';

    rc = bt_set_name(name);
    if (rc) {
        return BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    board_refresh_display();

    return 0;
}

static int
gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                    struct ble_gatt_access_ctxt *ctxt,
                    void *arg)
{
    const ble_uuid_t *uuid;
    int rc;

    uuid = ctxt->chr->uuid;

    if (ble_uuid_cmp(uuid, &name_enc_uuid.u) == 0) {
        switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            rc = read_name(ctxt->om);
            return rc;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            rc = write_name(ctxt->om);
            return rc;

        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
        }
    } else if (ble_uuid_cmp(uuid, &gatt_cud_uuid.u) == 0) {
        rc = os_mbuf_append(ctxt->om, "Badge Name",
                            (uint16_t) strlen("Badge Name"));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    } else if (ble_uuid_cmp(uuid, &gatt_cpf_uuid.u) == 0) {
        rc = os_mbuf_append(ctxt->om, &name_cpf,
                            (uint16_t) sizeof(name_cpf));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    /* Unknown characteristic; the nimble stack should not have called this
     * function.
     */
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

void
gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        MODLOG_DFLT(DEBUG, "registering characteristic %s with "
                           "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}

int
gatt_svr_init(void)
{
    int rc;

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}
