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
#include "mcu/mcu.h"

#if MYNEWT_VAL(BLE_HOST)
void
cmac_diag_setup_host(void)
{
    /* Setup pins for diagnostic signals */
    mcu_gpio_set_pin_function(42, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAG0); /* DIAG_0 @ P1.10 */
    mcu_gpio_set_pin_function(43, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAG1); /* DIAG_1 @ P1.11 */
    mcu_gpio_set_pin_function(44, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAG2); /* DIAG_2 @ P1.12 */
    mcu_gpio_set_pin_function(24, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_3 @ P0.24 */
    mcu_gpio_set_pin_function(21, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_4 @ P0.21 */
    mcu_gpio_set_pin_function(20, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_5 @ P0.20 */
    mcu_gpio_set_pin_function(19, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_6 @ P0.19 */
    mcu_gpio_set_pin_function(18, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_7 @ P0.18 */
    mcu_gpio_set_pin_function(31, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_8 @ P0.31 */
    mcu_gpio_set_pin_function(30, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_9 @ P0.30 */
    mcu_gpio_set_pin_function(29, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_10 @ P0.29 */
    mcu_gpio_set_pin_function(28, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_11 @ P0.28 */
    mcu_gpio_set_pin_function(27, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_12 @ P0.27 */
    mcu_gpio_set_pin_function(26, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_13 @ P0.26 */
    mcu_gpio_set_pin_function(38, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_14 @ P1.06 */
    mcu_gpio_set_pin_function(41, MCU_GPIO_MODE_OUTPUT, MCU_GPIO_FUNC_CMAC_DIAGX); /* DIAG_15 @ P1.09 */
}
#endif

#if MYNEWT_VAL(BLE_CONTROLLER)
void
cmac_diag_setup_cmac(void)
{
    MCU_DIAG_MAP( 0, 4, DSER);
    MCU_DIAG_MAP( 1, 6, CMAC_ON_ERROR);
    MCU_DIAG_MAP( 2, 2, PHY_TX_EN);
    MCU_DIAG_MAP( 3, 2, PHY_RX_EN);
    MCU_DIAG_MAP( 4, 2, PHY_TXRX_DATA_COMB);
    MCU_DIAG_MAP( 5, 2, PHY_TXRX_DATA_EN_COMB);
    MCU_DIAG_MAP( 6, 5, EV1US_FRAME_START);
    MCU_DIAG_MAP( 7, 5, EV_BS_START);
    MCU_DIAG_MAP( 8, 5, EV1C_BS_STOP);
    MCU_DIAG_MAP( 9, 5, EV1US_PHY_TO_IDLE);
    MCU_DIAG_MAP(10, 9, CALLBACK_IRQ);
    MCU_DIAG_MAP(11, 9, FIELD_IRQ);
    MCU_DIAG_MAP(12, 9, FRAME_IRQ);
    MCU_DIAG_MAP(13, 3, SLP_TIMER_ACTIVE);
    MCU_DIAG_MAP(14, 4, SLEEPING);
    MCU_DIAG_MAP(15, 8, LL_TIMER1_00);
}
#endif
