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
#include "syscfg/syscfg.h"
#include "mcu/mcu.h"
#include "os/os_arch.h"
#include "os/os.h"

#if MYNEWT_VAL(BLE_HOST)
int
cmac_rand_is_active(void)
{
    return g_cmac_shared_data->rand.cmr_active;
}

int
cmac_rand_is_full(void)
{
    int next;
    bool rc;

    next = cmac_rand_get_next();
    if (next == g_cmac_shared_data->rand.cmr_out) {
        rc = 1;
    } else {
        rc = 0;
    }
    return rc;
}

int
cmac_rand_get_next(void)
{
    int next;

    /* If active and not full, put event on queue to get random numbers */
    next = g_cmac_shared_data->rand.cmr_in + 1;
    if (next == CMAC_RAND_BUF_ELEMS) {
        next = 0;
    }
    return next;
}

void
cmac_rand_fill(uint32_t *buf, int num_words)
{
    int next;

    /* XXX: if words is 0, is it possible we could get into a state
       where we are waiting for random numbers but M33 does not know it
       has to fill any? */

    /* NOTE: we already know the buffer is not full first time through */
    next = g_cmac_shared_data->rand.cmr_in;
    while (num_words) {
        g_cmac_shared_data->rand.cmr_buf[next] = buf[0];
        next = cmac_rand_get_next();
        g_cmac_shared_data->rand.cmr_in = next;
        next = cmac_rand_get_next();
        if (next == g_cmac_shared_data->rand.cmr_out) {
            break;
        }
        --num_words;
        ++buf;
    }
}
#endif

#if MYNEWT_VAL(BLE_CONTROLLER)
static cmac_rand_isr_cb_t g_cmac_rand_isr_cb;

void
cmac_rand_set_isr_cb(cmac_rand_isr_cb_t cb)
{
    g_cmac_rand_isr_cb = cb;
}

void
cmac_rand_start(void)
{
    g_cmac_shared_data.rand.cmr_active = 1;
}

void
cmac_rand_stop(void)
{
    g_cmac_shared_data.rand.cmr_active = 0;
}

/**
 * cmac rnum read
 *
 * Called during the system to cmac isr to take random numbers
 * from shared memory into the BLE stack.
 */
void
cmac_rand_read(void)
{
    uint8_t bytes_left;
    uint32_t rnum;

    /* Just leave if no callback. */
    if (g_cmac_rand_isr_cb == NULL) {
        return;
    }

    bytes_left = 0;
    while (g_cmac_shared_data.rand.cmr_active) {
        if (bytes_left) {
            --bytes_left;
            rnum >>= 8;
        } else if (g_cmac_shared_data.rand.cmr_out != g_cmac_shared_data.rand.cmr_in) {
            bytes_left = 3;
            rnum = g_cmac_shared_data.rand.cmr_buf[g_cmac_shared_data.rand.cmr_out];
            ++g_cmac_shared_data.rand.cmr_out;
            if (g_cmac_shared_data.rand.cmr_out == CMAC_RAND_BUF_ELEMS) {
                g_cmac_shared_data.rand.cmr_out = 0;
            }
        } else {
            break;
        }
        (*g_cmac_rand_isr_cb)((uint8_t)rnum);
    }
}
#endif
