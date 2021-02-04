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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "syscfg/syscfg.h"
#include "sysflash/sysflash.h"
#include "os/os.h"
#include "mcu/mcu.h"
#include "mcu/cmsis_nvic.h"
#include "mcu/da1469x_hal.h"
#include "mcu/da1469x_lpclk.h"
#include "mcu/da1469x_clock.h"
#include "mcu/da1469x_trimv.h"
#include "mcu/da1469x_pdc.h"
#include "cmac_driver/cmac_host.h"
#include "cmac_driver/cmac_shared.h"
#include "cmac_driver/cmac_diag.h"
#include "trng/trng.h"
#if MYNEWT_VAL(CMAC_DEBUG_COREDUMP_ENABLE)
#include "console/console.h"
#endif

/* CMAC data */
extern char _binary_cmac_rom_bin_start[];
extern char _binary_cmac_rom_bin_end;
extern char _binary_cmac_ram_bin_start[];
extern char _binary_cmac_ram_bin_end;

struct cmac_image_info {
    uint32_t magic;
    uint32_t size_rom;
    uint32_t size_ram;
    uint32_t offset_data;
    uint32_t offset_shared;
};

/* PDC entry for waking up CMAC */
static int8_t g_cmac_host_pdc_sys2cmac;
/* PDC entry for waking up M33 */
static int8_t g_cmac_host_pdc_cmac2sys;

static void cmac_host_rand_fill(struct os_event *ev);
static struct os_event g_cmac_host_rand_ev = {
    .ev_cb = cmac_host_rand_fill
};

static void cmac_host_rand_chk_fill(void);

static void
cmac2sys_isr(void)
{
#if MYNEWT_VAL(CMAC_DEBUG_COREDUMP_ENABLE)
    volatile struct cmac_coredump *cd = &g_cmac_shared_data->coredump;
    const char *assert_file;
#endif

    os_trace_isr_enter();

    /* Clear CMAC2SYS interrupt */
    *(volatile uint32_t *)0x40002000 = 2;

    cmac_mbox_read();

    if (*(volatile uint32_t *)0x40002000 & 0x1c00) {
#if MYNEWT_VAL(CMAC_DEBUG_COREDUMP_ENABLE)
        console_blocking_mode();
        console_printf("CMAC error (0x%08lx)\n", *(volatile uint32_t *)0x40002000);
        console_printf("  lr:0x%08lx  pc:0x%08lx\n", cd->lr, cd->pc);
        if (cd->assert) {
            console_printf("  assert:0x%08lx\n", cd->assert);
            if (cd->assert_file) {
                /* Need to translate pointer from M0 code segment to M33 data */
                assert_file = cd->assert_file + MCU_MEM_SYSRAM_START_ADDRESS +
                              MEMCTRL->CMI_CODE_BASE_REG;
                console_printf("         %s:%d\n",
                               assert_file, (unsigned)cd->assert_line);
            }
        }
        console_printf("  0x%08lx CM_ERROR_REG\n", cd->CM_ERROR_REG);
        console_printf("  0x%08lx CM_EXC_STAT_REG\n", cd->CM_EXC_STAT_REG);
        console_printf("  0x%08lx CM_LL_TIMER1_36_10_REG\n", cd->CM_LL_TIMER1_36_10_REG);
        console_printf("  0x%08lx CM_LL_TIMER1_9_0_REG\n", cd->CM_LL_TIMER1_9_0_REG);

        /* Spin if debugger is connected to CMAC to avoid resetting it */
        if (cd->CM_STAT_REG & 0x20) {
            for (;;);
        }
#endif
        /* XXX CMAC is in error state, need to recover */
        assert(0);
        return;
    }

    cmac_host_rand_chk_fill();

    os_trace_isr_exit();
}

static void
cmac_host_rand_fill(struct os_event *ev)
{
    size_t num_bytes;
    struct trng_dev *trng;
    uint32_t *rnum;
    uint32_t rnums[CMAC_RAND_BUF_ELEMS];

    /* Check if full */
    if (!cmac_rand_is_active() || cmac_rand_is_full()) {
        return;
    }

    assert(ev->ev_arg != NULL);

    /* Fill buffer with random numbers even though we may not use all of them */
    trng = ev->ev_arg;
    rnum = &rnums[0];
    num_bytes = trng_read(trng, rnum, CMAC_RAND_BUF_ELEMS * sizeof(uint32_t));

    cmac_rand_fill(rnum, num_bytes / 4);
    cmac_host_signal2cmac();
}

static void
cmac_host_rand_chk_fill(void)
{
    if (cmac_rand_is_active() && !cmac_rand_is_full()) {
        os_eventq_put(os_eventq_dflt_get(), &g_cmac_host_rand_ev);
    }
}

void
cmac_host_signal2cmac(void)
{
    da1469x_pdc_set(g_cmac_host_pdc_sys2cmac);
}

static void
cmac_host_lpclk_cb(uint32_t freq)
{
    /* No need to wakeup CMAC if LP clock frequency did not change */
    if (g_cmac_shared_data->lp_clock_freq == freq) {
        return;
    }

    cmac_shared_lock();
    g_cmac_shared_data->lp_clock_freq = freq;
    g_cmac_shared_data->pending_ops |= CMAC_PENDING_OP_LP_CLK;
    cmac_shared_unlock();

    cmac_host_signal2cmac();
}

#if MYNEWT_VAL(CMAC_DEBUG_HOST_PRINT_ENABLE)
static void
cmac_host_print_trim(const char *name, const uint32_t *tv, unsigned len)
{
    console_printf("[CMAC] Trim values for '%s'\n", name);

    while (len) {
        console_printf("       0x%08x = 0x%08x\n", (unsigned)tv[0], (unsigned)tv[1]);
        len -= 2;
        tv += 2;
    }
}
#endif

void
cmac_host_rf_calibrate(void)
{
    cmac_shared_lock();
    g_cmac_shared_data->pending_ops |= CMAC_PENDING_OP_RF_CAL;
    cmac_shared_unlock();

    cmac_host_signal2cmac();
}

void
cmac_host_init(void)
{
    struct trng_dev *trng;
    struct cmac_image_info ii;
    uint32_t cmac_rom_size;
    uint32_t cmac_ram_size;
#if !MYNEWT_VAL(CMAC_IMAGE_SINGLE)
    const struct flash_area *fa;
    int rc;
#endif
    struct cmac_trim *trim;

    /* Get trng os device */
    trng = (struct trng_dev *) os_dev_open("trng", OS_TIMEOUT_NEVER, NULL);
    assert(trng);
    g_cmac_host_rand_ev.ev_arg = trng;

#if MYNEWT_VAL(CMAC_DEBUG_DIAG_ENABLE)
    cmac_diag_setup_host();
#endif

#if MYNEWT_VAL(CMAC_DEBUG_SWD_ENABLE)
    /* Enable CMAC debugger */
    CRG_TOP->SYS_CTRL_REG |= 0x40; /* CRG_TOP_SYS_CTRL_REG_CMAC_DEBUGGER_ENABLE_Msk */
#endif

    /*
     * Add PDC entry to wake up CMAC from M33
     *
     * XXX if MCU_DEBUG_GPIO_DEEP_SLEEP is enabled on CMAC, this should also
     *     enable PD_COM so CMAC can access GPIOs after wake up
     */
    g_cmac_host_pdc_sys2cmac = da1469x_pdc_add(MCU_PDC_TRIGGER_MAC_TIMER,
                                               MCU_PDC_MASTER_CMAC,
                                               MCU_PDC_EN_XTAL);
    da1469x_pdc_set(g_cmac_host_pdc_sys2cmac);
    da1469x_pdc_ack(g_cmac_host_pdc_sys2cmac);

    /* Add PDC entry to wake up M33 from CMAC, if does not exist yet */
    g_cmac_host_pdc_cmac2sys = da1469x_pdc_find(MCU_PDC_TRIGGER_COMBO,
                                              MCU_PDC_MASTER_M33, 0);
    if (g_cmac_host_pdc_cmac2sys < 0) {
        g_cmac_host_pdc_cmac2sys = da1469x_pdc_add(MCU_PDC_TRIGGER_COMBO,
                                                 MCU_PDC_MASTER_M33,
                                                 MCU_PDC_EN_XTAL);
        da1469x_pdc_set(g_cmac_host_pdc_cmac2sys);
        da1469x_pdc_ack(g_cmac_host_pdc_cmac2sys);
    }

    /* Setup CMAC2SYS interrupt */
    NVIC_SetVector(CMAC2SYS_IRQn, (uint32_t)cmac2sys_isr);
    NVIC_SetPriority(CMAC2SYS_IRQn, MYNEWT_VAL(CMAC_CMAC2SYS_IRQ_PRIORITY));
    NVIC_DisableIRQ(CMAC2SYS_IRQn);

    /* Enable Radio LDO */
    CRG_TOP->POWER_CTRL_REG |= CRG_TOP_POWER_CTRL_REG_LDO_RADIO_ENABLE_Msk;

    /* Enable CMAC, but keep it in reset */
    CRG_TOP->CLK_RADIO_REG = (1 << CRG_TOP_CLK_RADIO_REG_RFCU_ENABLE_Pos) |
                             (1 << CRG_TOP_CLK_RADIO_REG_CMAC_SYNCH_RESET_Pos) |
                             (0 << CRG_TOP_CLK_RADIO_REG_CMAC_CLK_SEL_Pos) |
                             (1 << CRG_TOP_CLK_RADIO_REG_CMAC_CLK_ENABLE_Pos) |
                             (0 << CRG_TOP_CLK_RADIO_REG_CMAC_DIV_Pos);

    /* Calculate size of ROM and RAM area */
    cmac_rom_size = &_binary_cmac_rom_bin_end - &_binary_cmac_rom_bin_start[0];
    cmac_ram_size = &_binary_cmac_ram_bin_end - &_binary_cmac_ram_bin_start[0];

    /* Load image header and check if image can be loaded */
#if MYNEWT_VAL(CMAC_IMAGE_SINGLE)
    memcpy(&ii, &_binary_cmac_rom_bin_start[128], sizeof(ii));
#else
    rc = flash_area_open(FLASH_AREA_IMAGE_1, &fa);
    assert(rc == 0);
    rc = flash_area_read(fa, 128, &ii, sizeof(ii));
    assert(rc == 0);
#endif

    assert(ii.magic == 0xC3ACC3AC);
    assert(ii.size_rom == cmac_rom_size);
    assert(ii.size_ram <= cmac_ram_size);

    /* Copy CMAC image to RAM */
#if MYNEWT_VAL(CMAC_IMAGE_SINGLE)
    memset(&_binary_cmac_ram_bin_start, 0xaa, cmac_ram_size);
    memcpy(&_binary_cmac_ram_bin_start, &_binary_cmac_rom_bin_start, ii.size_rom);
#else
    memset(&_binary_cmac_ram_bin_start, 0xaa, cmac_ram_size);
    rc = flash_area_read(fa, 0, &_binary_cmac_ram_bin_start, ii.size_rom);
    assert(rc == 0);
#endif

    /* Setup CMAC memory addresses */
    MEMCTRL->CMI_CODE_BASE_REG = (uint32_t)&_binary_cmac_ram_bin_start;
    MEMCTRL->CMI_DATA_BASE_REG = MEMCTRL->CMI_CODE_BASE_REG + ii.offset_data;
    MEMCTRL->CMI_SHARED_BASE_REG = MEMCTRL->CMI_CODE_BASE_REG + ii.offset_shared;
    MEMCTRL->CMI_END_REG = MEMCTRL->CMI_CODE_BASE_REG + ii.size_ram - 1;

    /* Initialize shared memory */
    cmac_shared_init();

    trim = (struct cmac_trim *)&g_cmac_shared_data->trim;
    trim->rfcu_len = da1469x_trimv_group_read(6, trim->rfcu, ARRAY_SIZE(trim->rfcu));
    trim->rfcu_mode1_len = da1469x_trimv_group_read(8, trim->rfcu_mode1, ARRAY_SIZE(trim->rfcu_mode1));
    trim->rfcu_mode2_len = da1469x_trimv_group_read(10, trim->rfcu_mode2, ARRAY_SIZE(trim->rfcu_mode2));
    trim->synth_len = da1469x_trimv_group_read(7, trim->synth, ARRAY_SIZE(trim->synth));

#if MYNEWT_VAL(CMAC_DEBUG_HOST_PRINT_ENABLE)
    cmac_host_print_trim("rfcu", trim->rfcu, trim->rfcu_len);
    cmac_host_print_trim("rfcu_mode1", trim->rfcu_mode1, trim->rfcu_mode1_len);
    cmac_host_print_trim("rfcu_mode2", trim->rfcu_mode2, trim->rfcu_mode2_len);
    cmac_host_print_trim("synth", trim->synth, trim->synth_len);
#endif

    /* Release CMAC from reset and sync */
    CRG_TOP->CLK_RADIO_REG &= ~CRG_TOP_CLK_RADIO_REG_CMAC_SYNCH_RESET_Msk;
    cmac_shared_sync();

    da1469x_lpclk_register_cmac_cb(cmac_host_lpclk_cb);

#if MYNEWT_VAL(CMAC_DEBUG_HOST_PRINT_ENABLE) && MYNEWT_VAL(CMAC_DEBUG_DATA_ENABLE)
    /* Trim values are calculated on RF init, so are valid after synced with CMAC */
    console_printf("[CMAC] Calculated trim_val1: 1=0x%08x 2=0x%08x\n",
                   (unsigned)g_cmac_shared_data->debug.trim_val1_tx_1,
                   (unsigned)g_cmac_shared_data->debug.trim_val1_tx_2);
    console_printf("[CMAC] Calculated trim_val2: tx=0x%08x rx=0x%08x\n",
                   (unsigned)g_cmac_shared_data->debug.trim_val2_tx,
                   (unsigned)g_cmac_shared_data->debug.trim_val2_rx);
#endif
}
