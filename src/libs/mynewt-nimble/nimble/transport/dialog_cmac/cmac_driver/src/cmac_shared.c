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

#include "syscfg/syscfg.h"
#if MYNEWT_VAL(BLE_CONTROLLER) && !MYNEWT_VAL(MCU_DEBUG_DSER_CMAC_SHARED)
#define MCU_DIAG_SER_DISABLE
#endif

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "mcu/mcu.h"
#include <cmac_driver/cmac_shared.h>
#include "os/os_arch.h"
#include "os/os.h"

#ifndef min
#define min(_a, _b)     ((_a) < (_b) ? (_a) : (_b))
#endif

#if MYNEWT_VAL(BLE_HOST)
volatile struct cmac_shared_data *g_cmac_shared_data;
#include "mcu/da1469x_clock.h"
#define MCU_DIAG_SER(_x)
#elif MYNEWT_VAL(BLE_CONTROLLER)
volatile struct cmac_shared_data g_cmac_shared_data     __attribute__((section(".shdata")));
#endif

void
cmac_shared_init(void)
{
#if MYNEWT_VAL(BLE_HOST)
    g_cmac_shared_data = (void *)(MCU_MEM_SYSRAM_START_ADDRESS +
                                  MEMCTRL->CMI_SHARED_BASE_REG);

    memset((void *)g_cmac_shared_data, 0, sizeof(*g_cmac_shared_data));

    g_cmac_shared_data->xtal32m_settle_us = MYNEWT_VAL(MCU_CLOCK_XTAL32M_SETTLE_TIME_US);

    g_cmac_shared_data->dcdc.enabled = DCDC->DCDC_CTRL1_REG & DCDC_DCDC_CTRL1_REG_DCDC_ENABLE_Msk;
    if (g_cmac_shared_data->dcdc.enabled) {
        g_cmac_shared_data->dcdc.v18 = DCDC->DCDC_V18_REG;
        g_cmac_shared_data->dcdc.v18p = DCDC->DCDC_V18P_REG;
        g_cmac_shared_data->dcdc.vdd = DCDC->DCDC_VDD_REG;
        g_cmac_shared_data->dcdc.v14 = DCDC->DCDC_V14_REG;
        g_cmac_shared_data->dcdc.ctrl1 = DCDC->DCDC_CTRL1_REG;
    }

#if MYNEWT_VAL(CMAC_DEBUG_DATA_ENABLE)
    g_cmac_shared_data->debug.tx_power_override = INT8_MAX;
#endif
#endif
}


void
cmac_shared_sync(void)
{
    /*
     * We need to guarantee proper order of initialization here, i.e. SYS has
     * to wait until CMAC finished initialization as otherwise host may start
     * sending HCI packets which will timeout as there is no one to read them.
     */
#if MYNEWT_VAL(BLE_HOST)
    assert(g_cmac_shared_data->magic_sys == 0);

    while (g_cmac_shared_data->magic_cmac != CMAC_SHARED_MAGIC_CMAC);
    g_cmac_shared_data->magic_sys = CMAC_SHARED_MAGIC_SYS;

    NVIC_EnableIRQ(CMAC2SYS_IRQn);
#endif

#if MYNEWT_VAL(BLE_CONTROLLER)
    assert(g_cmac_shared_data.magic_cmac == 0);

    g_cmac_shared_data.magic_cmac = CMAC_SHARED_MAGIC_CMAC;
    while (g_cmac_shared_data.magic_sys != CMAC_SHARED_MAGIC_SYS);

    NVIC_SetPriority(SYS2CMAC_IRQn, 3);
    NVIC_EnableIRQ(SYS2CMAC_IRQn);
#endif
}
