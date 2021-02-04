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
#include "mcu/mcu.h"
#include "mcu/cmac_timer.h"
#include "controller/ble_phy.h"
#include "cmac_driver/cmac_shared.h"
#include "ble_rf_priv.h"

#define RF_CALIBRATION_0        (0x01)
#define RF_CALIBRATION_1        (0x02)
#define RF_CALIBRATION_2        (0x04)

static const int8_t g_ble_rf_power_lvls[] = {
    -18, -12, -8, -6, -3, -2, -1, 0, 1, 2, 3, 4, 4, 5, 6
};

struct ble_phy_rf_data {
    uint8_t tx_power_cfg0;
    uint8_t tx_power_cfg1;
    uint8_t tx_power_cfg2;
    uint8_t tx_power_cfg3;
    uint32_t cal_res_1;
    uint32_t cal_res_2;
    uint32_t trim_val1_tx_1;
    uint32_t trim_val1_tx_2;
    uint32_t trim_val2_tx;
    uint32_t trim_val2_rx;
    uint8_t calibrate_req;
};

static struct ble_phy_rf_data g_ble_phy_rf_data;

static inline uint32_t
get_reg32(uint32_t addr)
{
    volatile uint32_t *reg = (volatile uint32_t *)addr;

    return *reg;
}

static inline uint32_t
get_reg32_bits(uint32_t addr, uint32_t mask)
{
    volatile uint32_t *reg = (volatile uint32_t *)addr;

    return (*reg & mask) >> __builtin_ctz(mask);
}

static inline void
set_reg8(uint32_t addr, uint8_t val)
{
    volatile uint8_t *reg = (volatile uint8_t *)addr;

    *reg = val;
}

static inline void
set_reg16(uint32_t addr, uint16_t val)
{
    volatile uint16_t *reg = (volatile uint16_t *)addr;

    *reg = val;
}

static inline void
set_reg32(uint32_t addr, uint32_t val)
{
    volatile uint32_t *reg = (volatile uint32_t *)addr;

    *reg = val;
}

static inline void
set_reg32_bits(uint32_t addr, uint32_t mask, uint32_t val)
{
    volatile uint32_t *reg = (volatile uint32_t *)addr;

    *reg = (*reg & (~mask)) | (val << __builtin_ctz(mask));
}

static inline void
set_reg32_mask(uint32_t addr, uint32_t mask, uint32_t val)
{
    volatile uint32_t *reg = (volatile uint32_t *)addr;

    *reg = (*reg & (~mask)) | (val & mask);
}

static inline void
set_reg16_mask(uint32_t addr, uint16_t mask, uint16_t val)
{
    volatile uint16_t *reg = (volatile uint16_t *)addr;

    *reg = (*reg & (~mask)) | (val & mask);
}

static void
delay_us(uint32_t delay_us)
{
    while (delay_us--) {
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP();
    }
}

static void
ble_rf_apply_trim(volatile uint32_t *tv, unsigned len)
{
    while (len) {
        *(volatile uint32_t *)tv[0] = tv[1];
        len -= 2;
        tv += 2;
    }
}

static void
ble_rf_apply_calibration(void)
{
    set_reg32(0x40020094, g_ble_phy_rf_data.cal_res_1);
    if (g_ble_phy_rf_data.cal_res_2) {
        set_reg32_bits(0x40022018, 0xff800000, g_ble_phy_rf_data.cal_res_2);
        set_reg32_bits(0x40022018, 0x00007fc0, g_ble_phy_rf_data.cal_res_2);
    }
}

static inline void
ble_rf_ldo_on(void)
{
    set_reg8(0x40020004, 9);
}

static inline void
ble_rf_ldo_off(void)
{
    set_reg8(0x40020004, 0);
}

static inline void
ble_rf_rfcu_enable(void)
{
    set_reg32_bits(0x50000010, 0x00000020, 1);
}

static inline void
ble_rf_rfcu_disable(void)
{
    set_reg32_bits(0x50000010, 0x00000020, 0);
}

static void
ble_rf_rfcu_apply_recommended_settings(void)
{
    set_reg16_mask(0x400200a0, 0x0001, 0x0001);
    set_reg16_mask(0x40021020, 0x03f0, 0x02f5);
    set_reg32_mask(0x40021018, 0x001fffff, 0x005a5809);
    set_reg32_mask(0x4002101c, 0x00001e01, 0x0040128c);
    set_reg32_mask(0x40021004, 0xffffff1f, 0x64442404);
    set_reg32_mask(0x40021008, 0xfcfcffff, 0x6b676665);
    set_reg32_mask(0x4002100c, 0x00fcfcfc, 0x9793736f);
    set_reg32_mask(0x40021010, 0x1f1f1c1f, 0x04072646);
    set_reg32_mask(0x40020000, 0x001ff000, 0x0f099820);
    set_reg16_mask(0x40020348, 0x00ff, 0x0855);
    set_reg16(0x40020350, 0x0234);
    set_reg16(0x40020354, 0x0a34);
    set_reg16(0x40020358, 0x0851);
    set_reg16(0x4002035c, 0x0a26);
    set_reg16(0x40020360, 0x0858);
    set_reg16(0x4002102c, 0xdfe7);
    set_reg32_mask(0x4002103c, 0x00c00000, 0x0024a19f);
    set_reg16_mask(0x40021000, 0x0008, 0x000b);
    set_reg16_mask(0x40020238, 0x03e0, 0x02c0);
    set_reg16_mask(0x4002023c, 0x03e0, 0x02c0);
    set_reg16_mask(0x40020244, 0x03e0, 0x0250);
    set_reg16_mask(0x40020248, 0x03e0, 0x02a0);
    set_reg16_mask(0x4002024c, 0x03e0, 0x02c0);
    set_reg16_mask(0x40020288, 0x03e0, 0x0300);
    set_reg16_mask(0x4002029c, 0x001f, 0x0019);
    set_reg16_mask(0x4002003c, 0x6000, 0x0788);
    set_reg16_mask(0x40020074, 0x7f00, 0x2007);
    set_reg32_mask(0x40020080, 0x00333330, 0x00222224);
    set_reg32_mask(0x40020068, 0x00000f0f, 0x00000f0d);
}

static void
ble_rf_rfcu_apply_settings(void)
{
    ble_rf_apply_trim(g_cmac_shared_data.trim.rfcu,
                      g_cmac_shared_data.trim.rfcu_len);
    ble_rf_rfcu_apply_recommended_settings();
}

static inline void
ble_rf_synth_enable(void)
{
    set_reg8(0x40020005, 3);
}

static inline void
ble_rf_synth_disable(void)
{
    set_reg8(0x40020005, 0);
    __NOP();
    __NOP();
}

static bool
ble_rf_synth_is_enabled(void)
{
    return get_reg32_bits(0x40020004, 256);
}

static void
ble_rf_synth_apply_recommended_settings(void)
{
    set_reg32_mask(0x40022048, 0x0000000c, 0x000000d5);
    set_reg32_mask(0x40022050, 0x00000300, 0x00000300);
    set_reg16_mask(0x40022024, 0x0001, 0x0001);
}

static void
ble_rf_synth_apply_settings(void)
{
    ble_rf_apply_trim(g_cmac_shared_data.trim.synth,
                      g_cmac_shared_data.trim.synth_len);
    ble_rf_synth_apply_recommended_settings();
}

static void
ble_rf_calibration_0(void)
{
    uint32_t bkp[10];

    bkp[0] = get_reg32(0x40020208);
    bkp[1] = get_reg32(0x40020250);
    bkp[2] = get_reg32(0x40020254);
    bkp[3] = get_reg32(0x40021028);
    bkp[4] = get_reg32(0x40020020);
    bkp[5] = get_reg32(0x40020294);
    bkp[6] = get_reg32(0x4002103C);
    bkp[7] = get_reg32(0x400200A8);
    bkp[8] = get_reg32(0x40020000);
    bkp[9] = get_reg32(0x40022000);

    set_reg32_bits(0x40020000, 0x00000002, 0);
    set_reg32_bits(0x40022000, 0x00000001, 0);
    set_reg32_mask(0x4002103C, 0x00201c00, 0x00001c00);
    set_reg32_bits(0x400200A8, 0x00000001, 1);
    set_reg8(0x40020006, 1);
    set_reg32(0x40020208, 0);
    set_reg32(0x40020250, 0);
    set_reg32(0x40020254, 0);
    set_reg32(0x40021028, 0x00F8A494);
    set_reg32(0x40020020, 8);
    set_reg32(0x40020294, 0);
    set_reg32(0x40020024, 0);

    delay_us(5);
    if (get_reg32_bits(0x40020020, 0x00000002)) {
        goto done;
    }

    set_reg32_bits(0x40020020, 0x00000001, 1);
    delay_us(15);
    if (!get_reg32_bits(0x40020020, 0x00000001)) {
        goto done;
    }

    delay_us(300);
    if (get_reg32_bits(0x40020020, 0x00000001)) {
        goto done;
    }

done:
    set_reg32(0x40020024, 0);
    set_reg32(0x40020208, bkp[0]);
    set_reg32(0x40020250, bkp[1]);
    set_reg32(0x40020254, bkp[2]);
    set_reg32(0x40021028, bkp[3]);
    set_reg32(0x40020020, bkp[4]);
    set_reg32(0x40020294, bkp[5]);
    set_reg32(0x4002103C, bkp[6]);
    set_reg32(0x400200A8, bkp[7]);
    set_reg32(0x40020000, bkp[8]);
    set_reg32(0x40022000, bkp[9]);
}

static void
ble_rf_calibration_1(void)
{
    uint32_t bkp[12];
    uint32_t val;

    bkp[0] = get_reg32(0x40020020);
    bkp[1] = get_reg32(0x40020208);
    bkp[2] = get_reg32(0x40020250);
    bkp[3] = get_reg32(0x40020254);
    bkp[4] = get_reg32(0x40020218);
    bkp[5] = get_reg32(0x4002021c);
    bkp[6] = get_reg32(0x40020220);
    bkp[7] = get_reg32(0x40020270);
    bkp[8] = get_reg32(0x4002027c);
    bkp[9] = get_reg32(0x4002101c);
    bkp[10] = get_reg32(0x40020000);
    bkp[11] = get_reg32(0x40022000);

    set_reg32(0x4002103c, 0x0124a21f);
    set_reg32(0x40020208, 0);
    set_reg32(0x40020250, 0);
    set_reg32(0x40020254, 0);
    set_reg32(0x40020218, 0);
    set_reg32(0x4002021c, 0);
    set_reg32(0x40020220, 0);
    set_reg32(0x40020270, 0);
    set_reg32(0x4002027c, 0);
    set_reg32(0x40020000, 0x0f168820);
    set_reg32_bits(0x40022000, 0x00000001, 0);
    set_reg32_bits(0x4002101c, 0x00001e00, 0);
    set_reg32_bits(0x4002001c, 0x0000003f, 47);
    set_reg8(0x40020006, 1);
    set_reg32(0x40020020, 16);
    set_reg32_bits(0x4002003c, 0x00000800, 1);
    set_reg32(0x40020024, 0);

    delay_us(5);
    if (get_reg32_bits(0x40020020, 0x00000002)) {
        goto done;
    }

    set_reg32_bits(0x40020020, 0x00000001, 1);
    delay_us(15);
    if (!get_reg32_bits(0x40020020, 0x00000001)) {
        goto done;
    }

    delay_us(300);
    if (get_reg32_bits(0x40020020, 0x00000001)) {
        goto done;
    }

    val = get_reg32(0x40020090);
    set_reg32_bits(0x40020094, 0x0000000f, val);
    set_reg32_bits(0x40020094, 0x00000f00, val);
    set_reg32_bits(0x40020094, 0x000f0000, val);
    set_reg32_bits(0x40020094, 0x0f000000, val);
    g_ble_phy_rf_data.cal_res_1 = get_reg32(0x40020094);

done:
    set_reg32(0x40020024, 0);
    set_reg32(0x40020020, bkp[0]);
    set_reg32(0x40020208, bkp[1]);
    set_reg32(0x40020250, bkp[2]);
    set_reg32(0x40020254, bkp[3]);
    set_reg32(0x40020218, bkp[4]);
    set_reg32(0x4002021c, bkp[5]);
    set_reg32(0x40020220, bkp[6]);
    set_reg32(0x40020270, bkp[7]);
    set_reg32(0x4002027c, bkp[8]);
    set_reg32(0x4002101c, bkp[9]);
    set_reg32(0x40020000, bkp[10]);
    set_reg32(0x40022000, bkp[11]);
    set_reg32_bits(0x4002003c, 0x00000800, 0);
}

static void
ble_rf_calibration_2(void)
{
    uint32_t bkp[2];
    uint32_t k1;

    set_reg8(0x40020005, 3);
    set_reg32(0x40022000, 0x00000300);
    set_reg32_bits(0x40022004, 0x0000007f, 20);
    bkp[0] = get_reg32(0x40022040);
    set_reg32(0x40022040, 0xffffffff);
    set_reg32_bits(0x40022018, 0x0000003f, 0);
    set_reg32_bits(0x40022018, 0x00008000, 0);
    set_reg32_bits(0x4002201c, 0x00000600, 2);
    set_reg32_bits(0x4002201c, 0x00000070, 4);
    set_reg32_bits(0x40022030, 0x3f000000, 22);
    set_reg32_bits(0x40022030, 0x00000fc0, 24);
    set_reg32_bits(0x40022030, 0x0000003f, 24);
    set_reg8(0x4002201c, 0x43);
    set_reg8(0x40020006, 2);
    delay_us(2);
    bkp[1] = get_reg32_bits(0x4002024c, 0x000003e0);
    set_reg32_bits(0x4002024c, 0x000003e0, 0);
    set_reg8(0x40020006, 1);
    set_reg32_bits(0x400200ac, 0x00000003, 3);
    delay_us(30);
    delay_us(100);
    set_reg8(0x40020005, 3);
    k1 = get_reg32_bits(0x40022088, 0x000001ff);
    set_reg32(0x400200ac, 0);
    delay_us(20);
    set_reg32_bits(0x4002024c, 0x000003e0, bkp[1]);
    delay_us(10);

    set_reg32_bits(0x40022018, 0xff800000, k1);
    set_reg32_bits(0x40022018, 0x00007fc0, k1);
    set_reg8(0x4002201c, 0x41);
    set_reg32_bits(0x4002201c, 0x00000600, 2);
    set_reg8(0x40020006, 2);
    delay_us(2);
    bkp[1] = get_reg32_bits(0x4002024c, 0x000003e0);
    set_reg32_bits(0x4002024c, 0x000003e0, 0);
    set_reg8(0x40020006, 1);
    set_reg32_bits(0x400200ac, 0x00000003, 3);
    delay_us(30);
    delay_us(100);
    set_reg8(0x40020005, 3);
    k1 = get_reg32_bits(0x40022088, 0x1ff);
    set_reg32(0x400200ac, 0);
    delay_us(20);
    set_reg32_bits(0x4002024c, 0x000003e0, bkp[1]);
    delay_us(10);

    set_reg32_bits(0x40022018, 0xff800000, k1);
    set_reg32_bits(0x40022018, 0x00007fc0, k1);
    set_reg8(0x4002201c, 0x41);
    set_reg32_bits(0x4002201c, 0x00000600, 2);
    set_reg8(0x40020006, 2);
    delay_us(2);
    bkp[1] = get_reg32_bits(0x4002024c, 0x000003e0);
    set_reg32_bits(0x4002024c, 0x000003e0, 0);
    set_reg8(0x40020006, 1);
    set_reg32_bits(0x400200ac, 0x00000003, 3);
    delay_us(30);
    delay_us(100);
    set_reg8(0x40020005, 3);
    k1 = get_reg32_bits(0x40022088, 0x000001ff);
    set_reg32_bits(0x40022018, 0xff800000, k1);
    set_reg32_bits(0x40022018, 0x00007fc0, k1);
    set_reg32_bits(0x4002201c, 0x00000001, 0);
    set_reg32(0x40022040, bkp[0]);
    set_reg32_bits(0x40022018, 0x0000003f, 0x1c);
    set_reg32_bits(0x40022018, 0x00008000, 0);
    set_reg32_bits(0x40022030, 0x3f000000, 28);
    set_reg32_bits(0x40022030, 0x00000fc0, 30);
    set_reg32_bits(0x40022030, 0x0000003f, 30);
    set_reg32(0x400200ac, 0);
    delay_us(20);
    set_reg32_bits(0x4002024c, 0x000003e0, bkp[1]);
    delay_us(10);

    g_ble_phy_rf_data.cal_res_2 = k1;
}

static void
ble_rf_calibrate_int(uint8_t mask)
{
    __disable_irq();

    ble_rf_enable();
    delay_us(20);

    ble_rf_synth_disable();
    ble_rf_synth_enable();
    ble_rf_synth_apply_settings();
    set_reg8(0x40020005, 1);

    if (mask & RF_CALIBRATION_0) {
        ble_rf_calibration_0();
    }
    if (mask & RF_CALIBRATION_1) {
        ble_rf_calibration_1();
    }
    if (mask & RF_CALIBRATION_2) {
        ble_rf_calibration_2();
    }

    ble_rf_disable();

    __enable_irq();

#if MYNEWT_VAL(CMAC_DEBUG_DATA_ENABLE)
    g_cmac_shared_data.debug.cal_res_1 = g_ble_phy_rf_data.cal_res_1;
    g_cmac_shared_data.debug.cal_res_2 = g_ble_phy_rf_data.cal_res_2;
#endif
}

bool
ble_rf_try_recalibrate(uint32_t idle_time_us)
{
    /* Run recalibration if we have at least 1ms of time to spare and RF is
     * currently disabled. Calibration is much shorter than 1ms, but that gives
     * us good margin to make sure we can finish before next event.
     */
    if (!g_ble_phy_rf_data.calibrate_req || (idle_time_us < 1000) ||
        ble_rf_is_enabled()) {
        return false;
    }

    ble_rf_calibrate_int(RF_CALIBRATION_2);

    g_ble_phy_rf_data.calibrate_req = 0;

    return true;
}

static uint32_t
ble_rf_find_trim_reg(volatile uint32_t *tv, unsigned len, uint32_t reg)
{
    while (len) {
        if (tv[0] == reg) {
            return tv[1];
        }
        len -= 2;
        tv += 2;
    }

    return 0;
}

void
ble_rf_init(void)
{
    static bool done = false;
    uint32_t val;

    ble_rf_disable();

    if (done) {
        return;
    }

    val = ble_rf_find_trim_reg(g_cmac_shared_data.trim.rfcu_mode1,
                               g_cmac_shared_data.trim.rfcu_mode1_len,
                               0x4002004c);
    g_ble_phy_rf_data.trim_val1_tx_1 = val;

    val = ble_rf_find_trim_reg(g_cmac_shared_data.trim.rfcu_mode2,
                               g_cmac_shared_data.trim.rfcu_mode2_len,
                               0x4002004c);
    g_ble_phy_rf_data.trim_val1_tx_2 = val;

    if (!g_ble_phy_rf_data.trim_val1_tx_1 || !g_ble_phy_rf_data.trim_val1_tx_2) {
        val = ble_rf_find_trim_reg(g_cmac_shared_data.trim.rfcu,
                                   g_cmac_shared_data.trim.rfcu_len,
                                   0x4002004c);
        if (!val) {
            val = 0x0300;
        }
        g_ble_phy_rf_data.trim_val1_tx_1 = val;
        g_ble_phy_rf_data.trim_val1_tx_2 = val;
    }

    val = ble_rf_find_trim_reg(g_cmac_shared_data.trim.synth,
                               g_cmac_shared_data.trim.synth_len,
                               0x40022038);
    if (!val) {
        val = 0x0198ff03;
    }
    g_ble_phy_rf_data.trim_val2_rx = val;
    g_ble_phy_rf_data.trim_val2_tx = val;
    set_reg32_bits((uint32_t)&g_ble_phy_rf_data.trim_val2_tx, 0x0001ff00, 0x87);

#if MYNEWT_VAL(CMAC_DEBUG_DATA_ENABLE)
    g_cmac_shared_data.debug.trim_val1_tx_1 = g_ble_phy_rf_data.trim_val1_tx_1;
    g_cmac_shared_data.debug.trim_val1_tx_2 = g_ble_phy_rf_data.trim_val1_tx_2;
    g_cmac_shared_data.debug.trim_val2_tx = g_ble_phy_rf_data.trim_val2_tx;
    g_cmac_shared_data.debug.trim_val2_rx = g_ble_phy_rf_data.trim_val2_rx;
#endif

    ble_rf_rfcu_enable();
    ble_rf_rfcu_apply_settings();
    g_ble_phy_rf_data.tx_power_cfg1 = get_reg32_bits(0x500000a4, 0xf0);
    g_ble_phy_rf_data.tx_power_cfg2 = get_reg32_bits(0x40020238, 0x000003e0);
    g_ble_phy_rf_data.tx_power_cfg3 = 0;
    ble_rf_rfcu_disable();

    ble_rf_calibrate_int(RF_CALIBRATION_0 | RF_CALIBRATION_1 | RF_CALIBRATION_2);

    done = true;
}

void
ble_rf_enable(void)
{
    if (ble_rf_is_enabled()) {
        return;
    }

    ble_rf_rfcu_enable();
    ble_rf_rfcu_apply_settings();
    ble_rf_ldo_on();
}

void
ble_rf_configure(void)
{
    if (ble_rf_synth_is_enabled()) {
        return;
    }

    ble_rf_synth_enable();
    ble_rf_synth_apply_settings();
}

void
ble_rf_stop(void)
{
    ble_rf_synth_disable();
    set_reg8(0x40020006, 0);
}

void
ble_rf_disable(void)
{
    ble_rf_stop();
    ble_rf_ldo_off();
    ble_rf_rfcu_disable();
}

bool
ble_rf_is_enabled(void)
{
    return get_reg32_bits(0x40020008, 5) == 5;
}

void
ble_rf_calibrate_req(void)
{
    g_ble_phy_rf_data.calibrate_req = 1;
}

void
ble_rf_setup_tx(uint8_t rf_chan, uint8_t phy_mode)
{
    set_reg32_bits(0x40020000, 0x0f000000, g_ble_phy_rf_data.tx_power_cfg0);
    set_reg32_bits(0x500000a4, 0x000000f0, g_ble_phy_rf_data.tx_power_cfg1);
    set_reg32_bits(0x40020238, 0x000003e0, g_ble_phy_rf_data.tx_power_cfg2);
    set_reg32_bits(0x40020234, 0x000003e0, g_ble_phy_rf_data.tx_power_cfg3);

    if (g_ble_phy_rf_data.tx_power_cfg0 < 13) {
        set_reg32(0x4002004c, g_ble_phy_rf_data.trim_val1_tx_1);
    } else {
        set_reg32(0x4002004c, g_ble_phy_rf_data.trim_val1_tx_2);
    }
    set_reg8(0x40020005, 3);
    set_reg8(0x40022004, rf_chan);
    if (phy_mode == BLE_PHY_MODE_2M) {
#if MYNEWT_VAL(BLE_PHY_RF_HP_MODE)
        set_reg32(0x40022000, 0x00000303);
#else
        set_reg32(0x40022000, 0x00000003);
#endif
    } else {
#if MYNEWT_VAL(BLE_PHY_RF_HP_MODE)
        set_reg32(0x40022000, 0x00000300);
#else
        set_reg32(0x40022000, 0x00000000);
#endif
    }

    ble_rf_apply_calibration();

    set_reg32_bits(0x40022050, 0x00000200, 1);
    set_reg32_bits(0x40022050, 0x00000100, 0);
    set_reg32_bits(0x40022048, 0x01ffff00, 0x7700);
    set_reg32(0x40022038, g_ble_phy_rf_data.trim_val2_tx);

    set_reg8(0x40020006, 3);
}

void
ble_rf_setup_rx(uint8_t rf_chan, uint8_t phy_mode)
{
    set_reg32_bits(0x500000a4, 0x000000f0, g_ble_phy_rf_data.tx_power_cfg1);
    set_reg8(0x40020005, 3);
    set_reg8(0x40022004, rf_chan);
    if (phy_mode == BLE_PHY_MODE_2M) {
#if MYNEWT_VAL(BLE_PHY_RF_HP_MODE)
        set_reg32(0x40022000, 0x00000303);
        set_reg32(0x40020000, 0x0f11b823);
        set_reg32(0x4002103c, 0x0125261b);
#else
        set_reg32(0x40022000, 0x00000003);
        set_reg32(0x40020000, 0x0f0c2803);
        set_reg32(0x4002103c, 0x0125a61b);
#endif
        set_reg32(0x40021020, 0x000002f5);
        set_reg32(0x4002102c, 0x0000d1d5);
    } else {
#if MYNEWT_VAL(BLE_PHY_RF_HP_MODE)
        set_reg32(0x40022000, 0x00000300);
        set_reg32(0x40020000, 0x0f099820);
        set_reg32(0x4002103c, 0x0124a21f);
#else
        set_reg32(0x40022000, 0x00000000);
        set_reg32(0x40020000, 0x0f062800);
        set_reg32(0x4002103c, 0x01051e1f);
#endif
        set_reg32(0x40021020, 0x000002f5);
        set_reg32(0x4002102c, 0x0000dfe7);
    }

    ble_rf_apply_calibration();

    set_reg32_bits(0x40022050, 0x00000200, 1);
    set_reg32_bits(0x40022050, 0x00000100, 1);
    set_reg32_bits(0x40022048, 0x01ffff00, 0);
    set_reg32(0x40022038, g_ble_phy_rf_data.trim_val2_rx);

    set_reg8(0x40020006, 3);
}

void
ble_rf_set_tx_power(int dbm)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(g_ble_rf_power_lvls); i++) {
        if (g_ble_rf_power_lvls[i] >= dbm) {
            break;
        }
    }

    g_ble_phy_rf_data.tx_power_cfg0 = i + 1;
}

int8_t
ble_rf_get_rssi(void)
{
    return (501 * get_reg32_bits(0x40021038, 0x000003ff) - 493000) / 4096;
}
