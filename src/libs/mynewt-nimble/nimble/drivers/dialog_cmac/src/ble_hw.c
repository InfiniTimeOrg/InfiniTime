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
#include <stdint.h>
#include "mcu/mcu.h"
#include "nimble/ble.h"
#include "controller/ble_hw.h"
#include "CMAC.h"
#include "cmac_driver/cmac_shared.h"
#include "mcu/mcu.h"
#include "tinycrypt/aes.h"

static struct tc_aes_key_sched_struct g_ctx;

int
ble_hw_rng_init(ble_rng_isr_cb_t cb, int bias)
{
    cmac_rand_set_isr_cb(cb);
    return 0;
}

int
ble_hw_rng_start(void)
{
    /* Chime the M33 in case we need random numbers generated */
    cmac_rand_start();
    CMAC->CM_EV_SET_REG = CMAC_CM_EV_SET_REG_EV1C_CMAC2SYS_IRQ_SET_Msk;
    return 0;
}

int
ble_hw_rng_stop(void)
{
    cmac_rand_stop();
    return 0;
}

#define BLE_HW_RESOLV_LIST_SIZE     (MYNEWT_VAL(BLE_LL_RESOLV_LIST_SIZE))

struct ble_hw_resolv_irk {
    uint32_t key[4];
};

struct ble_hw_resolv_list {
    uint8_t count;
    struct ble_hw_resolv_irk irk[BLE_HW_RESOLV_LIST_SIZE];
};

struct ble_hw_resolv_proc {
    uint32_t hash;
    uint8_t f_configured;
    uint8_t f_active;
    uint8_t f_match;
    uint8_t f_done;
    struct ble_hw_resolv_irk *irk;
    struct ble_hw_resolv_irk *irk_end;
    uint32_t crypto_prand_in[4];
    uint32_t crypto_e_out[4];
};

static struct ble_hw_resolv_list g_ble_hw_resolv_list;
static struct ble_hw_resolv_proc g_ble_hw_resolv_proc;

int
ble_hw_get_public_addr(ble_addr_t *addr)
{
    return -1;
}

int
ble_hw_get_static_addr(ble_addr_t *addr)
{
    return -1;
}

void
ble_hw_whitelist_clear(void)
{
}

int
ble_hw_whitelist_add(const uint8_t *addr, uint8_t addr_type)
{
    return 0;
}

void
ble_hw_whitelist_rmv(const uint8_t *addr, uint8_t addr_type)
{
}

uint8_t
ble_hw_whitelist_size(void)
{
    return 0;
}

void
ble_hw_whitelist_enable(void)
{
}


void
ble_hw_whitelist_disable(void)
{
}

int
ble_hw_whitelist_match(void)
{
    return 0;
}

int
ble_hw_encrypt_block(struct ble_encryption_block *ecb)
{
    uint32_t in_addr;
    uint32_t out_addr;

    /*
     * The following code bears some explanation. This function is called by
     * the LL task to encrypt blocks and calculate session keys. Address
     * resolution also calls this function. Furthermore, during connections,
     * the M0 crypto accelerator is used but this function is not called when
     * using it. During the entire connection event, the M0 crypto block cannot
     * be used as the crypto state (some of it) needs to remain un-changed.
     * Note that this is also true when address resolution is enabled: the
     * HW crypto block is set up and cannot be modified.
     *
     * Rather than attempt to share the M0 crypto block between the various
     * controller features which require it, we decided to use software to
     * perform the encryption task for anything being done at the link-layer
     * (outside of an ISR). If this function is called inside an ISR, and it
     * is when resolving addresses, the crypto accelerator is not being used
     * by a connection event. Thus, we check to see if we are inside of an ISR.
     * If so, we use the M0 crypto block. If outside of an ISR, we use the M33
     */
    if (!os_arch_in_isr()) {
        tc_aes128_set_encrypt_key(&g_ctx, ecb->key);
        tc_aes_encrypt(ecb->cipher_text, ecb->plain_text, &g_ctx);
        return 0;
    }

    /* Need to retain state of in/out pointers */
    in_addr = CMAC->CM_CRYPTO_IN_ADR2_REG;
    out_addr = CMAC->CM_CRYPTO_OUT_ADR_REG;

    while (CMAC->CM_CRYPTO_STAT_REG & CMAC_CM_CRYPTO_STAT_REG_CM_CRYPTO_BUSY_Msk);

    /* RECB, memory in/out, encryption */
    CMAC->CM_CRYPTO_CTRL_REG = CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_ECB_ENC_EN_Msk |
                               CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_IN_SEL_Msk |
                               CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_OUT_SEL_Msk |
                               CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_ENC_DECN_Msk;

    CMAC->CM_CRYPTO_KEY_31_0_REG = get_le32(&ecb->key[0]);
    CMAC->CM_CRYPTO_KEY_63_32_REG = get_le32(&ecb->key[4]);
    CMAC->CM_CRYPTO_KEY_95_64_REG = get_le32(&ecb->key[8]);
    CMAC->CM_CRYPTO_KEY_127_96_REG = get_le32(&ecb->key[12]);
    CMAC->CM_CRYPTO_IN_ADR2_REG = (uint32_t)ecb->plain_text;
    CMAC->CM_CRYPTO_OUT_ADR_REG = (uint32_t)ecb->cipher_text;

    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_CRYPTO_Msk;
    CMAC->CM_EV_SET_REG = CMAC_CM_EV_SET_REG_EV_CRYPTO_START_Msk;
    while (!(CMAC->CM_EXC_STAT_REG & CMAC_CM_EXC_STAT_REG_EXC_CRYPTO_Msk));
    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_CRYPTO_Msk;

    CMAC->CM_CRYPTO_IN_ADR2_REG = in_addr;
    CMAC->CM_CRYPTO_OUT_ADR_REG = out_addr;

    return 0;
}

void
ble_hw_resolv_list_clear(void)
{
    g_ble_hw_resolv_list.count = 0;
}

int
ble_hw_resolv_list_add(uint8_t *irk)
{
    struct ble_hw_resolv_irk *e;

    if (g_ble_hw_resolv_list.count == BLE_HW_RESOLV_LIST_SIZE) {
        return BLE_ERR_MEM_CAPACITY;
    }

    e = &g_ble_hw_resolv_list.irk[g_ble_hw_resolv_list.count];
    /* Prepare key here so we do not need to do it during resolving */
    e->key[0] = get_le32(&irk[0]);
    e->key[1] = get_le32(&irk[4]);
    e->key[2] = get_le32(&irk[8]);
    e->key[3] = get_le32(&irk[12]);

    g_ble_hw_resolv_list.count++;

    return BLE_ERR_SUCCESS;
}

void
ble_hw_resolv_list_rmv(int index)
{
    struct ble_hw_resolv_irk *e;

    if (index < g_ble_hw_resolv_list.count) {
        g_ble_hw_resolv_list.count--;

        e = &g_ble_hw_resolv_list.irk[index];
        memmove(e, e + 1, (g_ble_hw_resolv_list.count - index) * sizeof(e->key));
    }
}

uint8_t
ble_hw_resolv_list_size(void)
{
    return BLE_HW_RESOLV_LIST_SIZE;
}

int
ble_hw_resolv_list_match(void)
{
    return g_ble_hw_resolv_proc.f_match ?
           g_ble_hw_resolv_proc.irk - g_ble_hw_resolv_list.irk : -1;
}

static void
ble_hw_resolv_proc_next(void)
{
    void *src = &g_ble_hw_resolv_proc.irk->key;

    if (g_ble_hw_resolv_proc.irk == g_ble_hw_resolv_proc.irk_end) {
        g_ble_hw_resolv_proc.f_done = 1;
        g_ble_hw_resolv_proc.f_active = 0;
    } else {
        __asm__ volatile (".syntax unified                      \n"
                          "   ldm  %[ptr]!, {r1, r2, r3, r4}    \n"
                          "   ldr  %[ptr], =%[reg]              \n"
                          "   stm  %[ptr]!, {r1, r2, r3, r4}    \n"
                          : [ptr] "+l" (src)
                          : [reg] "i" (&CMAC->CM_CRYPTO_KEY_31_0_REG)
                          : "r1", "r2", "r3", "r4", "memory");

        CMAC->CM_EV_SET_REG = CMAC_CM_EV_SET_REG_EV_CRYPTO_START_Msk;
    }
}

void
ble_hw_resolv_proc_enable(void)
{
    assert(!g_ble_hw_resolv_proc.f_active);

    CMAC->CM_CRYPTO_CTRL_REG = CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_SW_REQ_ABORT_Msk;

    CMAC->CM_CRYPTO_CTRL_REG = CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_ECB_ENC_EN_Msk |
                               CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_IN_SEL_Msk |
                               CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_OUT_SEL_Msk |
                               CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_ENC_DECN_Msk;

    CMAC->CM_CRYPTO_IN_ADR2_REG = (uint32_t)g_ble_hw_resolv_proc.crypto_prand_in;
    CMAC->CM_CRYPTO_OUT_ADR_REG = (uint32_t)g_ble_hw_resolv_proc.crypto_e_out;

    g_ble_hw_resolv_proc.irk = g_ble_hw_resolv_list.irk;
    g_ble_hw_resolv_proc.irk_end = g_ble_hw_resolv_list.irk +
                                   g_ble_hw_resolv_list.count;
    g_ble_hw_resolv_proc.f_configured = 1;
    g_ble_hw_resolv_proc.f_active = 0;

    /*
     * It would be better to enable IRQ in ble_hw_resolv_proc_start, but this
     * would introduce a bit of latency when starting resolving procedure and
     * we need to save every us possible there in order to be able to resolve
     * RPA on time.
     */
    NVIC_ClearPendingIRQ(CRYPTO_IRQn);
    NVIC_EnableIRQ(CRYPTO_IRQn);
}

void
ble_hw_resolv_proc_disable(void)
{
    g_ble_hw_resolv_proc.f_configured = 0;
    g_ble_hw_resolv_proc.f_active = 0;
    g_ble_hw_resolv_proc.f_match = 0;
    g_ble_hw_resolv_proc.f_done = 1;

    NVIC_DisableIRQ(CRYPTO_IRQn);
}

void
ble_hw_resolv_proc_start(const uint8_t *addr)
{
    assert(g_ble_hw_resolv_proc.f_configured);

    /* crypto_prand_in is already zeroed so prand is properly padded */
    g_ble_hw_resolv_proc.crypto_prand_in[3] = get_be24(&addr[3]) << 8;
    g_ble_hw_resolv_proc.hash = get_be24(&addr[0]);

    g_ble_hw_resolv_proc.f_match = 0;
    g_ble_hw_resolv_proc.f_done = 0;
    g_ble_hw_resolv_proc.f_active = 1;

    ble_hw_resolv_proc_next();
}

void
CRYPTO_IRQHandler(void)
{
    uint32_t hash;

    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_CRYPTO_Msk;

    hash = g_ble_hw_resolv_proc.crypto_e_out[3] >> 8;
    if (g_ble_hw_resolv_proc.hash == hash) {
        g_ble_hw_resolv_proc.f_active = 0;
        g_ble_hw_resolv_proc.f_match = 1;
        g_ble_hw_resolv_proc.f_done = 1;
    } else {
        g_ble_hw_resolv_proc.irk++;
        ble_hw_resolv_proc_next();
    }
}
