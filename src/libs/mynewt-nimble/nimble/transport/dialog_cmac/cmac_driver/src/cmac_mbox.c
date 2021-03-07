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
#include <cmac_driver/cmac_shared.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "syscfg/syscfg.h"
#include "mcu/mcu.h"
#include "os/os_arch.h"
#include "os/os.h"

#ifndef min
#define min(_a, _b)     ((_a) < (_b) ? (_a) : (_b))
#endif

static cmac_mbox_read_cb *g_cmac_mbox_read_cb;
static cmac_mbox_write_notif_cb *g_cmac_mbox_write_notif_cb;

void
cmac_mbox_set_read_cb(cmac_mbox_read_cb *cb)
{
    g_cmac_mbox_read_cb = cb;
}

void
cmac_mbox_set_write_notif_cb(cmac_mbox_write_notif_cb *cb)
{
    g_cmac_mbox_write_notif_cb = cb;
}

int
cmac_mbox_read(void)
{
#if MYNEWT_VAL(BLE_HOST)
    volatile struct cmac_mbox *mbox = &g_cmac_shared_data->mbox_c2s;
    uint8_t *mbox_buf = (uint8_t *)&g_cmac_shared_data->mbox_c2s_buf;
    const uint16_t mbox_size = MYNEWT_VAL(CMAC_MBOX_SIZE_C2S);
#else
    volatile struct cmac_mbox *mbox = &g_cmac_shared_data.mbox_s2c;
    uint8_t *mbox_buf = (uint8_t *)&g_cmac_shared_data.mbox_s2c_buf;
    const uint16_t mbox_size = MYNEWT_VAL(CMAC_MBOX_SIZE_S2C);
#endif
    uint16_t rd_off;
    uint16_t wr_off;
    uint16_t chunk;
    int len = 0;

    if (!g_cmac_mbox_read_cb) {
        return 0;
    }

    do {
        rd_off = mbox->rd_off;
        wr_off = mbox->wr_off;

        if (rd_off <= wr_off) {
            chunk = wr_off - rd_off;
        } else {
            chunk = mbox_size - rd_off;
        }

        while (chunk) {
            len = g_cmac_mbox_read_cb(&mbox_buf[rd_off], chunk);
            if (len < 0) {
                break;
            }

            rd_off += len;
            chunk -= len;
        }

        mbox->rd_off = rd_off == mbox_size ? 0 : rd_off;
    } while ((mbox->rd_off != mbox->wr_off) && (len >= 0));

    return 0;
}

int
cmac_mbox_write(const void *data, uint16_t len)
{
#if MYNEWT_VAL(BLE_HOST)
    volatile struct cmac_mbox *mbox = &g_cmac_shared_data->mbox_s2c;
    uint8_t *mbox_buf = (uint8_t *)&g_cmac_shared_data->mbox_s2c_buf;
    const uint16_t mbox_size = MYNEWT_VAL(CMAC_MBOX_SIZE_S2C);
#else
    volatile struct cmac_mbox *mbox = &g_cmac_shared_data.mbox_c2s;
    uint8_t *mbox_buf = (uint8_t *)&g_cmac_shared_data.mbox_c2s_buf;
    const uint16_t mbox_size = MYNEWT_VAL(CMAC_MBOX_SIZE_C2S);
#endif
    uint16_t rd_off;
    uint16_t wr_off;
    uint16_t max_wr;
    uint16_t chunk;

    while (len) {
        rd_off = mbox->rd_off;
        wr_off = mbox->wr_off;

        /*
         * Calculate maximum length to write, i.e. up to end of buffer or stop
         * before rd_off to be able to detect full queue.
         */
        if (rd_off > wr_off) {
            /*
             * |0|1|2|3|4|5|6|7|
             * | | | |W| | |R| |
             *        `---^
             */
            max_wr = rd_off - wr_off - 1;
        } else if (rd_off == 0) {
            /*
             * |0|1|2|3|4|5|6|7|
             * |R| | |W| | | | |
             *        `-------^
             */
            max_wr = mbox_size - wr_off - 1;
        } else {
            /*
             * |0|1|2|3|4|5|6|7|
             * | |R| |W| | | | |
             *        `---------^
             */
            max_wr = mbox_size - wr_off;
        }

        chunk = min(len, max_wr);

        if (chunk == 0) {
            continue;
        }

        memcpy(&mbox_buf[wr_off], data, chunk);

        wr_off += chunk;
        mbox->wr_off = wr_off == mbox_size ? 0 : wr_off;

        g_cmac_mbox_write_notif_cb();

        len -= chunk;
        data = (uint8_t *)data + chunk;
    }

    return 0;
}
