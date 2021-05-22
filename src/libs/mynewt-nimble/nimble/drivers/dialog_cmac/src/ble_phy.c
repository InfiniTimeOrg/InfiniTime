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
#if !MYNEWT_VAL(BLE_PHY_DEBUG_DSER)
#define MCU_DIAG_SER_DISABLE
#endif

#include <assert.h>
#include <stdint.h>
#include <assert.h>
#include "nimble/ble.h"
#include "mcu/mcu.h"
#include "mcu/cmac_timer.h"
#include "cmac_driver/cmac_shared.h"
#include "controller/ble_phy.h"
#include "controller/ble_ll.h"
#include "stats/stats.h"
#include "CMAC.h"
#include "ble_hw_priv.h"
#include "ble_rf_priv.h"

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_CODED_PHY)
#error LE Coded PHY cannot be enabled on DA1469x
#endif

/* Statistics */
STATS_SECT_START(ble_phy_stats)
    STATS_SECT_ENTRY(phy_isrs)
    STATS_SECT_ENTRY(tx_good)
    STATS_SECT_ENTRY(tx_fail)
    STATS_SECT_ENTRY(tx_late)
    STATS_SECT_ENTRY(tx_late_sched)
    STATS_SECT_ENTRY(tx_late_frame)
    STATS_SECT_ENTRY(tx_late_field)
    STATS_SECT_ENTRY(tx_bytes)
    STATS_SECT_ENTRY(rx_starts)
    STATS_SECT_ENTRY(rx_aborts)
    STATS_SECT_ENTRY(rx_valid)
    STATS_SECT_ENTRY(rx_crc_err)
    STATS_SECT_ENTRY(rx_late)
    STATS_SECT_ENTRY(radio_state_errs)
    STATS_SECT_ENTRY(rx_hw_err)
    STATS_SECT_ENTRY(tx_hw_err)
STATS_SECT_END
STATS_SECT_DECL(ble_phy_stats) ble_phy_stats;

STATS_NAME_START(ble_phy_stats)
    STATS_NAME(ble_phy_stats, phy_isrs)
    STATS_NAME(ble_phy_stats, tx_good)
    STATS_NAME(ble_phy_stats, tx_fail)
    STATS_NAME(ble_phy_stats, tx_late)
    STATS_NAME(ble_phy_stats, tx_late_sched)
    STATS_NAME(ble_phy_stats, tx_late_frame)
    STATS_NAME(ble_phy_stats, tx_late_field)
    STATS_NAME(ble_phy_stats, tx_bytes)
    STATS_NAME(ble_phy_stats, rx_starts)
    STATS_NAME(ble_phy_stats, rx_aborts)
    STATS_NAME(ble_phy_stats, rx_valid)
    STATS_NAME(ble_phy_stats, rx_crc_err)
    STATS_NAME(ble_phy_stats, rx_late)
    STATS_NAME(ble_phy_stats, radio_state_errs)
    STATS_NAME(ble_phy_stats, rx_hw_err)
    STATS_NAME(ble_phy_stats, tx_hw_err)
STATS_NAME_END(ble_phy_stats)

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_CODED_PHY)
#error LE Coded PHY is not supported
#endif

/* An easy way to get and set bit field value in CMAC registers */
#define CMAC_SETREGF(_reg, _field, _val)                                    \
    CMAC->_reg = (CMAC->_reg & ~(CMAC_ ## _reg ## _ ## _field ## _Msk)) |   \
                 ((_val) << (CMAC_ ## _reg ## _ ## _field ## _Pos));
#define CMAC_GETREGF(_reg, _field)                                          \
    (CMAC->_reg & (CMAC_ ## _reg ## _ ## _field ## _Msk)) >>                \
    (CMAC_ ## _reg ## _ ## _field ## _Pos)

/* Definitions for fields queue */
#define FIELD_DATA_REG_DMA_TX(_offset, _len) \
    ((uint32_t)&g_ble_phy_tx_buf[(_offset)] & 0x3ffff) | ((_len) << 20)
#define FIELD_DATA_REG_DMA_RX(_offset, _len) \
    ((uint32_t)&g_ble_phy_rx_buf[(_offset)] & 0x3ffff) | ((_len) << 20)

#if MYNEWT_VAL(BLE_LL_DTM)
#define PHY_WHITENING   (g_ble_phy_data.phy_whitening)
#else
#define PHY_WHITENING   (1)
#endif

#define FIELD_CTRL_REG_TX_PREAMBLE                                  \
    (0 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_CRC_Pos) |           \
    (0 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_WHITENING_Pos) |     \
    (0 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_TX_DATA_SRC_Pos) |      \
    (g_ble_phy_data.phy_mode_evpsym <<                              \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EVPSYMBOL_LUT_Pos) |         \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_VALID_Pos) |            \
    (g_ble_phy_data.phy_mode_pre_len <<                             \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_SIZE_M1_Pos)
#define FIELD_CTRL_REG_TX_ACCESS_ADDR                               \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EXC_ON_EXP_Pos) |       \
    (0 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_CRC_Pos) |           \
    (0 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_WHITENING_Pos) |     \
    (0 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_TX_DATA_SRC_Pos) |      \
    (g_ble_phy_data.phy_mode_evpsym <<                              \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EVPSYMBOL_LUT_Pos) |         \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_VALID_Pos) |            \
    (31 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_SIZE_M1_Pos)
#define FIELD_CTRL_REG_TX_PAYLOAD                                   \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_CRC_Pos) |           \
    (PHY_WHITENING <<                                               \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_WHITENING_Pos) |          \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_DMA_MEM_Pos) |       \
    (g_ble_phy_data.phy_mode_evpsym <<                              \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EVPSYMBOL_LUT_Pos) |         \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_VALID_Pos);
#define FIELD_CTRL_REG_TX_ENC_PAYLOAD                               \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_CRC_Pos) |           \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_WHITENING_Pos) |     \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_DMA_CRYPTO_Pos) |    \
    (g_ble_phy_data.phy_mode_evpsym <<                              \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EVPSYMBOL_LUT_Pos) |         \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_VALID_Pos)
#define FIELD_CTRL_REG_TX_MIC                                       \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_CRC_Pos) |           \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_WHITENING_Pos) |     \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_DMA_CRYPTO_Pos) |    \
    (g_ble_phy_data.phy_mode_evpsym <<                              \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EVPSYMBOL_LUT_Pos) |         \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_VALID_Pos)
#define FIELD_CTRL_REG_TX_CRC                                       \
    (0 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_CRC_Pos) |           \
    (PHY_WHITENING <<                                               \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_WHITENING_Pos) |          \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_TX_DATA_SRC_Pos) |      \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_LAST_Pos) |             \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_MSB_FIRST_Pos) |        \
    (g_ble_phy_data.phy_mode_evpsym <<                              \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EVPSYMBOL_LUT_Pos) |         \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_VALID_Pos) |            \
    (23 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_SIZE_M1_Pos)
#define FIELD_CTRL_REG_RX_ACCESS_ADDR \
    (0 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_CRC_Pos) |           \
    (0 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_WHITENING_Pos) |     \
    (0 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_TX_DATA_SRC_Pos) |      \
    (g_ble_phy_data.phy_mode_evpsym <<                              \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EVPSYMBOL_LUT_Pos) |         \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_CORR_Pos) |          \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_VALID_Pos) |            \
    (31 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_SIZE_M1_Pos)
#define FIELD_CTRL_REG_RX_HEADER \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EXC_ON_EXP_Pos) |       \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_CRC_Pos) |           \
    (PHY_WHITENING <<                                               \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_WHITENING_Pos) |          \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_DMA_MEM_Pos) |       \
    (g_ble_phy_data.phy_mode_evpsym <<                              \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EVPSYMBOL_LUT_Pos) |         \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_VALID_Pos)
#define FIELD_CTRL_REG_RX_CRC                                       \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_CRC_Pos) |           \
    (PHY_WHITENING <<                                               \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_WHITENING_Pos) |          \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_LAST_Pos) |             \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_MSB_FIRST_Pos) |        \
    (g_ble_phy_data.phy_mode_evpsym <<                              \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EVPSYMBOL_LUT_Pos) |         \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_VALID_Pos) |            \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_DMA_MEM_Pos)
#define FIELD_CTRL_REG_RX_PAYLOAD                                   \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_CRC_Pos) |           \
    (PHY_WHITENING <<                                               \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_WHITENING_Pos) |          \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_DMA_MEM_Pos) |       \
    (g_ble_phy_data.phy_mode_evpsym <<                              \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EVPSYMBOL_LUT_Pos) |         \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_VALID_Pos)
#define FIELD_CTRL_REG_RX_PAYLOAD_WITH_EXC \
    FIELD_CTRL_REG_RX_PAYLOAD |                                     \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EXC_ON_EXP_Pos)
#define FIELD_CTRL_REG_RX_ENC_PAYLOAD                               \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_CRC_Pos) |           \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_WHITENING_Pos) |     \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_XL_DMA_CRYPTO_Pos) |    \
    (g_ble_phy_data.phy_mode_evpsym <<                              \
     CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_EVPSYMBOL_LUT_Pos) |         \
    (1 << CMAC_CM_FIELD_PUSH_CTRL_REG_FIELD_VALID_Pos)

/* RF power up/down delays */
#define PHY_DELAY_POWER_DN_RX   (23)
#define PHY_DELAY_POWER_DN_TX   (23)
#define PHY_DELAY_POWER_UP_RX   (90)
#define PHY_DELAY_POWER_UP_TX   (75)
#define PHY_DELAY_TX_RX         ((PHY_DELAY_POWER_DN_TX) + (PHY_DELAY_POWER_UP_RX))
#define PHY_DELAY_RX_TX         ((PHY_DELAY_POWER_DN_RX) + (PHY_DELAY_POWER_UP_TX))

/* RF TX/RX path delays */
static const uint8_t g_ble_phy_path_delay_tx[2] = {
    4, /* 1M = 3.8us */
    0, /* 2M = 0.2us */
};
static const uint8_t g_ble_phy_path_delay_rx[2] = {
    2, /* 1M = 2.2us */
    1, /* 2M = 0.8us */
};

/* Measured and pre-calculated offsets for transitions */
static const uint8_t g_ble_phy_frame_offset_txrx[4] = {
    ((BLE_LL_IFS) - (PHY_DELAY_TX_RX) + (4)), /* 2M/1M */
    ((BLE_LL_IFS) - (PHY_DELAY_TX_RX) + (5)), /* 1M/1M */
    ((BLE_LL_IFS) - (PHY_DELAY_TX_RX) + (4)), /* 2M/2M */
    ((BLE_LL_IFS) - (PHY_DELAY_TX_RX) + (5)), /* 1M/2M */
};
static const uint8_t g_ble_phy_frame_offset_rxtx[4] = {
    ((BLE_LL_IFS) - (PHY_DELAY_RX_TX) - (5)), /* 2M/1M */
    ((BLE_LL_IFS) - (PHY_DELAY_RX_TX) - (6)), /* 1M/1M */
    ((BLE_LL_IFS) - (PHY_DELAY_RX_TX) - (3)), /* 2M/2M */
    ((BLE_LL_IFS) - (PHY_DELAY_RX_TX) - (5)), /* 1M/2M */
};

/* packet start offsets (in usecs) */
static const uint16_t g_ble_phy_mode_pkt_start_off[BLE_PHY_NUM_MODE] = { 376, 40, 24, 376 };

struct ble_phy_data {
    uint8_t phy_state;          /* Current state */
    uint8_t channel;            /* Current PHY channel */
    uint8_t phy_mode_cur;       /* Current PHY mode */
    uint8_t phy_mode_tx;        /* TX PHY mode */
    uint8_t phy_mode_rx;        /* RX PHY mode */
    uint8_t phy_mode_pre_len;   /* Preamble length - 1 */
    uint8_t phy_mode_evpsym;    /* EVPSYMBOL_LUT value for fields */
    uint8_t end_transition;     /* Scheduled transition */
    uint8_t path_delay_tx;
    uint8_t path_delay_rx;
    uint8_t frame_offset_txrx;
    uint8_t frame_offset_rxtx;
    uint8_t phy_rx_started;
    uint8_t phy_encrypted;
    uint8_t phy_privacy;
#if MYNEWT_VAL(BLE_LL_DTM)
    uint8_t phy_whitening;      /* Whitening state (disabled for DTM) */
#endif
    uint32_t access_addr;       /* Current access address */
    uint32_t crc_init;
    uint32_t llt_at_cputime;
    uint32_t cputime_at_llt;
    uint64_t start_llt;
    struct ble_mbuf_hdr rxhdr;
    ble_phy_tx_end_func txend_cb;
    void *txend_arg;
};

static struct ble_phy_data g_ble_phy_data;

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
/* Encryption related variables */
struct ble_phy_encrypt_obj {
    uint8_t key[16];
    uint8_t b0[16];
    uint8_t b1[16];
    uint8_t ai[16];
};

struct ble_phy_encrypt_obj g_ble_phy_encrypt_data;

static void ble_phy_tx_enc_start(void);
static void ble_phy_rx_enc_start(uint8_t len);
#endif

#define SW_MAC_EXC_NONE             (0)
#define SW_MAC_EXC_LL_RX_END        (1)
#define SW_MAC_EXC_TXEND_CB         (2)
#define SW_MAC_EXC_LL_RX_START      (3)
#define SW_MAC_EXC_WFR_TIMER_EXP    (4)

static volatile uint8_t g_sw_mac_exc;

/* Channel index to RF channel mapping */
static const uint8_t g_ble_phy_chan_to_rf[BLE_PHY_NUM_CHANS] = {
     1,  2,  3,  4,  5,  6,  7,  8,  9, 10, /* 0-9 */
    11, 13, 14, 15, 16, 17, 18, 19, 20, 21, /* 10-19 */
    22, 23, 24, 25, 26, 27, 28, 29, 30, 31, /* 20-29 */
    32, 33, 34, 35, 36, 37, 38,  0, 12, 39, /* 30-39 */
};

__attribute__((aligned(4)))
static uint8_t g_ble_phy_tx_buf[BLE_PHY_MAX_PDU_LEN + 3];
__attribute__((aligned(4)))
static uint8_t g_ble_phy_rx_buf[BLE_PHY_MAX_PDU_LEN + 3];

static void ble_phy_irq_field_tx(void);
static void ble_phy_irq_field_rx(void);
static void ble_phy_irq_frame_tx(void);
static void ble_phy_irq_frame_rx(void);
static bool ble_phy_rx_start_isr(void);
static void ble_phy_rx_setup_fields(void);
static void ble_phy_rx_setup_xcvr(void);
static void ble_phy_mode_apply(uint8_t phy_mode);

void
FIELD_IRQHandler(void)
{
    MCU_DIAG_SER('E');

    switch (g_ble_phy_data.phy_state) {
    case BLE_PHY_STATE_TX:
        ble_phy_irq_field_tx();
        break;
    case BLE_PHY_STATE_RX:
        ble_phy_irq_field_rx();
        break;
    default:
        STATS_INC(ble_phy_stats, radio_state_errs);
        CMAC->CM_EXC_STAT_REG = 0xfffffffe;
        break;
    }

    MCU_DIAG_SER('e');
}

void
CALLBACK_IRQHandler(void)
{
    MCU_DIAG_SER('C');

    /* XXX: clear these for now. */
    (void)CMAC->CM_BS_SMPL_D_REG;

    /* Clear IRQ*/
    CMAC->CM_EV_SET_REG = CMAC_CM_EV_SET_REG_EV1C_CALLBACK_VALID_CLR_Msk;
    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_FIELD_ON_THR_EXP_Msk;

    /*
     * Program next frame for transition to TX. CM_EV_LINKUP_REG register to
     * enable actual transition can be set later, we just need to make sure 2nd
     * frame is already set before current frame is finished - this guarantees
     * that frame for transition will be moved to 1st frame once current frame
     * is popped off the queue.
     */
    CMAC->CM_FRAME_2_REG = CMAC_CM_FRAME_2_REG_FRAME_VALID_Msk |
                           CMAC_CM_FRAME_2_REG_FRAME_TX_Msk |
                           CMAC_CM_FRAME_2_REG_FRAME_EXC_ON_BS_START_Msk |
                           ((g_ble_phy_data.frame_offset_rxtx) <<
                            CMAC_CM_FRAME_2_REG_FRAME_START_OFFSET_Pos);

    /*
     * We just got an access address match so do this as early as possible
     * to save time in the field rx isr.
     */
    ble_phy_rx_start_isr();

    MCU_DIAG_SER('c');
}

void
FRAME_IRQHandler(void)
{
    MCU_DIAG_SER('F');

    switch (g_ble_phy_data.phy_state) {
    case BLE_PHY_STATE_TX:
        ble_phy_irq_frame_tx();
        break;
    case BLE_PHY_STATE_RX:
        ble_phy_irq_frame_rx();
        break;
    default:
        STATS_INC(ble_phy_stats, radio_state_errs);
        CMAC->CM_EXC_STAT_REG = 0xfffffffe;
        break;
    }

    MCU_DIAG_SER('f');
}

void
SW_MAC_IRQHandler(void)
{
    uint8_t exc;
    int rc;

    MCU_DIAG_SER('S');

    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_SW_MAC_Msk;
    assert(g_sw_mac_exc);

    exc = g_sw_mac_exc;
    g_sw_mac_exc = 0;

    MCU_DIAG_SER('0' + exc);

    /* Next SW_MAC handover can now be queued */
    os_arch_cmac_bs_ctrl_irq_unblock();

    switch (exc) {
    case SW_MAC_EXC_TXEND_CB:
        assert(g_ble_phy_data.txend_cb);
        g_ble_phy_data.txend_cb(g_ble_phy_data.txend_arg);
        break;
    case SW_MAC_EXC_WFR_TIMER_EXP:
        ble_ll_wfr_timer_exp(NULL);
        break;
    case SW_MAC_EXC_LL_RX_START:
        /* Call Link Layer receive start function */
        rc = ble_ll_rx_start(&g_ble_phy_rx_buf[0], g_ble_phy_data.channel,
                             &g_ble_phy_data.rxhdr);
        if (rc == 0) {
            /* Set rx started flag and enable rx end ISR */
            g_ble_phy_data.phy_rx_started = 1;

            /* No transition */
            CMAC->CM_EV_LINKUP_REG = CMAC_CM_EV_LINKUP_REG_LU_PHY_TO_IDLE_2_NONE_Msk |
                                     CMAC_CM_EV_LINKUP_REG_LU_FRAME_START_2_NONE_Msk;
        } else if (rc > 0) {
            g_ble_phy_data.phy_rx_started = 1;

            /* Setup transition */
            CMAC->CM_EV_LINKUP_REG = CMAC_CM_EV_LINKUP_REG_LU_PHY_TO_IDLE_2_EXC_Msk |
                                     CMAC_CM_EV_LINKUP_REG_LU_FRAME_START_2_PHY_TO_IDLE_Msk;
        } else {
            /* Disable PHY */
            ble_phy_disable();
            STATS_INC(ble_phy_stats, rx_aborts);
        }
        break;
    case SW_MAC_EXC_LL_RX_END:
        /* Call LL end processing */
        rc = ble_ll_rx_end(&g_ble_phy_rx_buf[0], &g_ble_phy_data.rxhdr);
        if (rc < 0) {
            ble_phy_disable();
        }
        break;
    default:
        assert(0);
        break;
    }

    MCU_DIAG_SER('s');
}

static inline uint32_t
ble_phy_convert_and_record_start_time(uint32_t cputime, uint8_t rem_usecs)
{
    uint64_t ll_val;

    ll_val = cmac_timer_convert_hal2llt(cputime);

    /*
     * Since we just converted cputime to the LL timer, record both these
     * values as they will be used to calculate packet reception start time.
     */
    g_ble_phy_data.cputime_at_llt = cputime;
    g_ble_phy_data.llt_at_cputime = ll_val;
    g_ble_phy_data.start_llt = ll_val + rem_usecs;

    return ll_val;
}

static inline void
ble_phy_sw_mac_handover(uint8_t exc)
{
    assert(!g_sw_mac_exc);

    g_sw_mac_exc = exc;

    CMAC->CM_EV_SET_REG = CMAC_CM_EV_SET_REG_EV1C_SW_MAC_Msk;

    /*
     * We want SW_MAC to be fired just after BS_CTRL interrupt so we block
     * BS_CTRL temporarily and SW_MAC is next in order of interrupts priority.
     */
    os_arch_cmac_bs_ctrl_irq_block();
}

static void
ble_phy_rx_end_isr(void)
{
    struct ble_mbuf_hdr *ble_hdr;

    /* XXX just clear captured timer for now. Handle rx end time */
    (void)CMAC->CM_TS1_REG;

    /* Set RSSI and CRC status flag in header */
    ble_hdr = &g_ble_phy_data.rxhdr;

    /* Count PHY crc errors and valid packets */
    if (CMAC->CM_CRC_REG != 0) {
        STATS_INC(ble_phy_stats, rx_crc_err);
    } else {
        STATS_INC(ble_phy_stats, rx_valid);
        ble_hdr->rxinfo.flags |= BLE_MBUF_HDR_F_CRC_OK;
#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
        if (g_ble_phy_data.phy_encrypted) {
            /* Only set MIC failure flag if frame is not zero length */
            if (g_ble_phy_rx_buf[1] != 0) {
                if (CMAC->CM_CRYPTO_STAT_REG != 0) {
                    ble_hdr->rxinfo.flags |= BLE_MBUF_HDR_F_MIC_FAILURE;
                } else {
                    g_ble_phy_rx_buf[1] = g_ble_phy_rx_buf[1] - 4;
                }
            }
        }
#endif
    }

    ble_phy_sw_mac_handover(SW_MAC_EXC_LL_RX_END);
}

static bool
ble_phy_rx_start_isr(void)
{
    uint32_t llt32;
    uint32_t llt_10_0;
    uint32_t llt_10_0_mask;
    uint32_t timestamp;
    uint32_t ticks;
    uint32_t usecs;
    struct ble_mbuf_hdr *ble_hdr;

    /* Initialize the ble mbuf header */
    ble_hdr = &g_ble_phy_data.rxhdr;
    ble_hdr->rxinfo.flags = ble_ll_state_get();
    ble_hdr->rxinfo.channel = g_ble_phy_data.channel;
    ble_hdr->rxinfo.handle = 0;
    ble_hdr->rxinfo.phy = ble_phy_get_cur_phy();
    ble_hdr->rxinfo.phy_mode = g_ble_phy_data.phy_mode_rx;
#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LL_EXT_ADV)
    ble_hdr->rxinfo.user_data = NULL;
#endif

    /* Read the latched RSSI value */
    ble_hdr->rxinfo.rssi = ble_rf_get_rssi();
#if MYNEWT_VAL(CMAC_DEBUG_DATA_ENABLE)
    g_cmac_shared_data.debug.last_rx_rssi = ble_hdr->rxinfo.rssi;
#endif

    /* Count rx starts */
    STATS_INC(ble_phy_stats, rx_starts);

    /*
     * Calculate packet start time. Note that we have only received the
     * access address at this point but we should have the 1st symbol and
     * thus the timestamp should be set (this is based on looking at the diag
     * signals). For now, lets make sure that the dirty bit is set. The
     * dirty bit means that the timestamp was set since the last time cleared.
     * Note that we need to read the timestamp first to guarantee it was set
     * before reading the LL timer.
     */
    timestamp = CMAC->CM_TS1_REG;
    assert((timestamp & CMAC_CM_TS1_REG_TS1_DIRTY_Msk) != 0);

    /* Get the LL timer (only need 32 bits) */
    llt32 = cmac_timer_read32();

    /*
     * We assume that the timestamp was set within 11 bits, or 2047 usecs, of
     * when we read the ll timer. We assume this because we need to calculate
     * the LL timer value at the timestamp. If the low 11 bits of the LL timer
     * are greater than the timestamp, it means that the upper bits of the
     * timestamp are correct. If the timestamp value is greater, it means the
     * timer wrapped the 11 bits and we need to adjust the LL timer value.
     */
    llt_10_0_mask = (CMAC_CM_TS1_REG_TS1_TIMER1_9_0_Msk |
                     CMAC_CM_TS1_REG_TS1_TIMER1_10_Msk);
    timestamp &= llt_10_0_mask;
    llt_10_0 = llt32 & llt_10_0_mask;
    llt32 &= ~llt_10_0_mask;
    if (timestamp > llt_10_0) {
        llt32 -= 2048;
    }
    llt32 |= timestamp;

    /* Actual RX start time needs to account for preamble and access address */
    llt32 -= g_ble_phy_mode_pkt_start_off[g_ble_phy_data.phy_mode_rx] +
             g_ble_phy_data.path_delay_rx;

    if (llt32 < g_ble_phy_data.llt_at_cputime) {
        g_ble_phy_data.llt_at_cputime -= 31;
        g_ble_phy_data.cputime_at_llt--;
    }

    /*
     * We now have the LL timer when the packet was received. Get the cputime
     * and the leftover usecs.
     */
    usecs = llt32 - g_ble_phy_data.llt_at_cputime;
    ticks = os_cputime_usecs_to_ticks(usecs);
    ble_hdr->beg_cputime = g_ble_phy_data.cputime_at_llt + ticks;
    ble_hdr->rem_usecs = usecs - os_cputime_ticks_to_usecs(ticks);

    return true;
}

static void
ble_phy_irq_field_tx_exc_bs_start_4this(void)
{
    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_BS_START_4THIS_Msk;

    if (g_ble_phy_data.end_transition == BLE_PHY_TRANSITION_TX_RX) {
        /*
         * Setup 2nd frame that will start after current one.
         * -2us offset to adjust for allowed active clock accuracy.
         */
        CMAC->CM_FRAME_2_REG = CMAC_CM_FRAME_2_REG_FRAME_VALID_Msk |
                               (((g_ble_phy_data.frame_offset_txrx) - 2) <<
                                CMAC_CM_FRAME_2_REG_FRAME_START_OFFSET_Pos);

        /* Next frame starts automatically on phy2idle */
        CMAC->CM_EV_LINKUP_REG = CMAC_CM_EV_LINKUP_REG_LU_PHY_TO_IDLE_2_EXC_Msk |
                                 CMAC_CM_EV_LINKUP_REG_LU_FRAME_START_2_PHY_TO_IDLE_Msk;

        ble_phy_wfr_enable(BLE_PHY_WFR_ENABLE_TXRX, g_ble_phy_data.phy_mode_rx, 0);
    } else {
        CMAC->CM_EV_LINKUP_REG = CMAC_CM_EV_LINKUP_REG_LU_PHY_TO_IDLE_2_EXC_Msk |
                                 CMAC_CM_EV_LINKUP_REG_LU_FRAME_START_2_NONE_Msk;
    }

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
    if (g_ble_phy_data.phy_encrypted && (g_ble_phy_tx_buf[1] != 0)) {
        ble_phy_tx_enc_start();
    }
#endif
}

static void
ble_phy_irq_field_tx_exc_field_on_thr_exp(void)
{
    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_FIELD_ON_THR_EXP_Msk;
    (void)CMAC->CM_TS1_REG;

    /*  Set up remaining field (CRC) */
    CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_TX_CRC;
}

static void
ble_phy_irq_field_tx(void)
{
    uint32_t stat;

    stat = CMAC->CM_EXC_STAT_REG;

    if (stat & CMAC_CM_EXC_STAT_REG_EXC_BS_START_4THIS_Msk) {
        MCU_DIAG_SER('6');
        ble_phy_irq_field_tx_exc_bs_start_4this();
    }

    if (stat & CMAC_CM_EXC_STAT_REG_EXC_FIELD_ON_THR_EXP_Msk) {
        MCU_DIAG_SER('7');
        ble_phy_irq_field_tx_exc_field_on_thr_exp();
    }
}

static void
ble_phy_irq_frame_tx_exc_bs_stop(void)
{
    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_BS_STOP_Msk;

    /* Clear latched timestamp so we do not have error on next frame */
    (void)CMAC->CM_TS1_REG;

    if (g_ble_phy_data.end_transition == BLE_PHY_TRANSITION_TX_RX) {
#if (BLE_LL_BT5_PHY_SUPPORTED == 1)
        ble_phy_mode_apply(g_ble_phy_data.phy_mode_rx);
#endif
        ble_phy_rx_setup_fields();
    } else {
        CMAC->CM_EV_LINKUP_REG = CMAC_CM_EV_LINKUP_REG_LU_PHY_TO_IDLE_2_EXC_Msk |
                                 CMAC_CM_EV_LINKUP_REG_LU_FRAME_START_2_NONE_Msk;
    }

    if (g_ble_phy_data.txend_cb) {
        ble_phy_sw_mac_handover(SW_MAC_EXC_TXEND_CB);
        return;
    }
}

static void
ble_phy_irq_frame_tx_exc_phy_to_idle_4this(void)
{
    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_PHY_TO_IDLE_4THIS_Msk;

    if (g_ble_phy_data.end_transition == BLE_PHY_TRANSITION_TX_RX) {
        ble_phy_rx_setup_xcvr();

        g_ble_phy_data.phy_state = BLE_PHY_STATE_RX;
    } else {
        /*
         * Disable explicitly in case RX-TX was done (we cannot setup for auto
         * disable in such case) */
        ble_rf_stop();

        g_ble_phy_data.phy_state = BLE_PHY_STATE_IDLE;
    }

    g_ble_phy_data.end_transition = BLE_PHY_TRANSITION_NONE;
}

static void
ble_phy_irq_frame_tx(void)
{
    uint32_t stat;

    stat = CMAC->CM_EXC_STAT_REG;

    /*
     * In case of phy2idle this should be first and only exception we handle
     * here. This is because in case of TX-RX transition frame_start will occur
     * at the same as phy2idle so we will have 2 exceptions here. To handle this
     * properly we first need to handle phy2idle in TX state and keep frame_start
     * pending so it will be called again in RX state.
     */
    if (stat & CMAC_CM_EXC_STAT_REG_EXC_PHY_TO_IDLE_4THIS_Msk) {
        MCU_DIAG_SER('6');
        ble_phy_irq_frame_tx_exc_phy_to_idle_4this();
        return;
    }

    if (stat & CMAC_CM_EXC_STAT_REG_EXC_FRAME_START_Msk) {
        MCU_DIAG_SER('7');
        CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_FRAME_START_Msk;
    }

    if (stat & CMAC_CM_EXC_STAT_REG_EXC_BS_STOP_Msk) {
        MCU_DIAG_SER('8');
        ble_phy_irq_frame_tx_exc_bs_stop();
    }
}

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
static void
ble_phy_field_rx_encrypted(uint32_t len)
{
    if (len) {
        /*
         * An encrypted frame should have a minimum length of 5
         * bytes (at least one for payload and 4 for MIC). If the
         * length is less than 5 this frame is bogus and will most
         * likely fail CRC. We still need to process this frame
         * though as we need to call the handover function with
         * the frame. If this happens we will not bother to
         * run the remaining bytes through the accelerator; just
         * process them like normal and generate (a hopefully
         * incorrect) CRC.
         */
        if (len >= 5) {
            /* Start the crypto accelerator */
            ble_phy_rx_enc_start(len);

            /*
             * We have already processed one byte; process remaining
             * payload and MIC. Note: length contains MIC.
             */
            len -= 2;
            CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_RX(4, len);
            CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_ENC_PAYLOAD;

            /* CRC */
            CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_RX(4 + len, 3);
            CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_CRC;
        } else {
            /* We have processed one byte so far. Send remaining
               payload bytes to normal rx payload processing */
            len -= 2;
            if (len) {
                CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_RX(4, len);
                CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_PAYLOAD;
            }

            CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_RX(4 + len, 3);
            CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_CRC;

            /* Clear crypto pre-buffer */
            CMAC->CM_CRYPTO_CTRL_REG = CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_SW_REQ_PBUF_CLR_Msk;
        }
    } else {
        /* We programmed one byte, so get next two bytes for CRC */
        CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_RX(4, 1);
        CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_CRC;
    }
}
#endif

static void
ble_phy_field_rx_unencrypted(uint32_t len)
{
    uint8_t pduhdr;
    uint8_t adva_thr;

    if (len) {
        pduhdr = g_ble_phy_rx_buf[0];
        adva_thr = 0;

        /*
         * Setup interrupt after AdvA to start address resolving if
         * privacy is enabled and TxAdd bit is set.
         */
        if (MYNEWT_VAL(BLE_LL_CFG_FEAT_LL_PRIVACY) &&
            g_ble_phy_data.phy_privacy && (pduhdr & 0x40)) {

            /*
             * For legacy advertising AdvA ends at 6th byte.
             * For extended advertising AdvA ends at 8th byte.
             * We already programmed 2 bytes of payload so need
             * to adjust threshold accordingly or just reset it
             * in case there is not enough bytes in PDU to fit AdvA.
             */
            adva_thr = (pduhdr & 0x0f) == 0x07 ? 6 : 4;
            if (len >= adva_thr + 2) {
                CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_RX(4, adva_thr);
                CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_PAYLOAD_WITH_EXC;
            } else {
                adva_thr = 0;
            }
        }

        CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_RX(4 + adva_thr,
                                                             len - adva_thr);
        CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_PAYLOAD;
    }

    CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_RX(4 + len, 1);
    CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_CRC;
}

static void
ble_phy_irq_field_rx_exc_field_on_thr_exp(void)
{
    uint32_t len;
    uint32_t smpl;

    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_FIELD_ON_THR_EXP_Msk;

    smpl = CMAC->CM_BS_SMPL_ST_REG;

    if ((smpl & CMAC_CM_BS_SMPL_ST_REG_FIELD_CNT_LATCHED_Msk) == 1) {
        assert(g_ble_phy_data.phy_rx_started == 0);

        /* Clear this */
        (void)CMAC->CM_TS1_REG;

        /* Read length of frame */
        len = CMAC->CM_BS_SMPL_D_REG;
        len = len & 0xFF;

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
        if (g_ble_phy_data.phy_encrypted) {
            ble_phy_field_rx_encrypted(len);
        } else {
            ble_phy_field_rx_unencrypted(len);
        }
#else
        ble_phy_field_rx_unencrypted(len);
#endif

        ble_phy_sw_mac_handover(SW_MAC_EXC_LL_RX_START);
    } else if ((smpl & CMAC_CM_BS_SMPL_ST_REG_FIELD_CNT_LATCHED_Msk) == 3) {
        (void)CMAC->CM_BS_SMPL_D_REG;

        assert(g_ble_phy_data.phy_privacy);

        /*
         * Resolve only if RPA is received. AdvA is at different offset
         * in ExtAdv PDU. TxAdd was already checked before programming
         * field threshold.
         */
        if ((g_ble_phy_rx_buf[0] & 0x0f) == 0x07) {
            if ((g_ble_phy_rx_buf[9] & 0xc0) == 0x40) {
                ble_hw_resolv_proc_start(&g_ble_phy_rx_buf[4]);
            }
        } else {
            if ((g_ble_phy_rx_buf[7] & 0xc0) == 0x40) {
                ble_hw_resolv_proc_start(&g_ble_phy_rx_buf[2]);
            }
        }
    } else {
        assert(0);
    }
}

static void
ble_phy_irq_field_rx_exc_stat_reg_exc_corr_timeout(void)
{
    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_CORR_TIMEOUT_Msk;
    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_BS_STOP_Msk;
    CMAC->CM_EV_LINKUP_REG = CMAC_CM_EV_LINKUP_REG_LU_FRAME_START_2_NONE_Msk;

    ble_phy_sw_mac_handover(SW_MAC_EXC_WFR_TIMER_EXP);
}

static void
ble_phy_irq_field_rx(void)
{
    uint32_t stat;

    stat = CMAC->CM_EXC_STAT_REG;

    if (stat & CMAC_CM_EXC_STAT_REG_EXC_FIELD_ON_THR_EXP_Msk) {
        MCU_DIAG_SER('1');
        ble_phy_irq_field_rx_exc_field_on_thr_exp();
    }

    if (stat & CMAC_CM_EXC_STAT_REG_EXC_CORR_TIMEOUT_Msk) {
        MCU_DIAG_SER('2');
        ble_phy_irq_field_rx_exc_stat_reg_exc_corr_timeout();
    }
}

static void
ble_phy_irq_frame_rx_exc_phy_to_idle_4this(void)
{
    uint8_t rf_chan;

    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_PHY_TO_IDLE_4THIS_Msk;

    /* We are here only on transition, so switch to TX */
#if (BLE_LL_BT5_PHY_SUPPORTED == 1)
    ble_phy_mode_apply(g_ble_phy_data.phy_mode_tx);
#endif
    rf_chan = g_ble_phy_chan_to_rf[g_ble_phy_data.channel];
    ble_rf_setup_tx(rf_chan, g_ble_phy_data.phy_mode_tx);
    g_ble_phy_data.phy_state = BLE_PHY_STATE_TX;
}

static void
ble_phy_irq_frame_rx_exc_frame_start(void)
{
    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_FRAME_START_Msk;
}

static void
ble_phy_irq_frame_rx_exc_bs_stop(void)
{
    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_BS_STOP_Msk;
    ble_phy_rx_end_isr();
}

static void
ble_phy_irq_frame_rx(void)
{
    uint32_t stat;

    stat = CMAC->CM_EXC_STAT_REG;

    if (stat & CMAC_CM_EXC_STAT_REG_EXC_PHY_TO_IDLE_4THIS_Msk) {
        MCU_DIAG_SER('3');
        ble_phy_irq_frame_rx_exc_phy_to_idle_4this();
        return;
    }

    if (stat & CMAC_CM_EXC_STAT_REG_EXC_FRAME_START_Msk) {
        MCU_DIAG_SER('1');
        ble_phy_irq_frame_rx_exc_frame_start();
    }

    if (stat & CMAC_CM_EXC_STAT_REG_EXC_BS_STOP_Msk) {
        MCU_DIAG_SER('2');
        ble_phy_irq_frame_rx_exc_bs_stop();
    }
}

static void
ble_phy_mode_apply(uint8_t phy_mode)
{
    if (phy_mode == g_ble_phy_data.phy_mode_cur) {
        return;
    }

    switch (phy_mode) {
    case BLE_PHY_MODE_1M:
#if MYNEWT_VAL(BLE_PHY_RF_HP_MODE)
        CMAC_SETREGF(CM_PHY_CTRL2_REG, CORR_CLK_MODE, 3);
#endif
        CMAC_SETREGF(CM_PHY_CTRL2_REG, PHY_MODE, 1);
        g_ble_phy_data.phy_mode_evpsym = 1; /* 1000 ns per symbol */
        g_ble_phy_data.phy_mode_pre_len = 7;
        break;
#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_2M_PHY)
    case BLE_PHY_MODE_2M:
#if MYNEWT_VAL(BLE_PHY_RF_HP_MODE)
        CMAC_SETREGF(CM_PHY_CTRL2_REG, CORR_CLK_MODE, 2);
#endif
        CMAC_SETREGF(CM_PHY_CTRL2_REG, PHY_MODE, 0);
        g_ble_phy_data.phy_mode_evpsym = 0; /* 500 ns per symbol */
        g_ble_phy_data.phy_mode_pre_len = 15;
        break;
#endif
    default:
        assert(0);
        return;
    }

    g_ble_phy_data.phy_mode_cur = phy_mode;
}

void
ble_phy_mode_set(uint8_t tx_phy_mode, uint8_t rx_phy_mode)
{
    uint8_t txrx;
    uint8_t rxtx;

    g_ble_phy_data.phy_mode_tx = tx_phy_mode;
    g_ble_phy_data.phy_mode_rx = rx_phy_mode;

    g_ble_phy_data.path_delay_tx = g_ble_phy_path_delay_tx[tx_phy_mode - 1];
    g_ble_phy_data.path_delay_rx = g_ble_phy_path_delay_rx[rx_phy_mode - 1];

    /*
     * Calculate index of transition in frame offset array without tons of
     * branches. Note that transitions have to be in specific order in array.
     *
     * phy_mode 1M = 01b
     * phy_mode 2M = 10b
     *
     * 1M/1M = 01b | 00b = 01b
     * 1M/2M = 01b | 10b = 11b
     * 2M/1M = 00b | 00b = 00b
     * 2M/2M = 00b | 10b = 10b
     */
    txrx = (tx_phy_mode & 0x01) | (rx_phy_mode & 0x02);
    rxtx = (rx_phy_mode & 0x01) | (tx_phy_mode & 0x02);
    g_ble_phy_data.frame_offset_txrx = g_ble_phy_frame_offset_txrx[txrx];
    g_ble_phy_data.frame_offset_rxtx = g_ble_phy_frame_offset_rxtx[rxtx];
}

int
ble_phy_get_cur_phy(void)
{
#if (BLE_LL_BT5_PHY_SUPPORTED == 1)
    switch (g_ble_phy_data.phy_mode_cur) {
    case BLE_PHY_MODE_1M:
        return BLE_PHY_1M;
    case BLE_PHY_MODE_2M:
        return BLE_PHY_2M;
    default:
        assert(0);
        return -1;
    }
#else
    return BLE_PHY_1M;
#endif
}

/**
 * Copies the data from the phy receive buffer into a mbuf chain.
 *
 * @param dptr Pointer to receive buffer
 * @param rxpdu Pointer to already allocated mbuf chain
 *
 * NOTE: the packet header already has the total mbuf length in it. The
 * lengths of the individual mbufs are not set prior to calling.
 *
 */
void
ble_phy_rxpdu_copy(uint8_t *dptr, struct os_mbuf *rxpdu)
{
    uint32_t rem_len;
    uint32_t copy_len;
    uint32_t block_len;
    uint32_t block_rem_len;
    void *dst;
    void *src;
    struct os_mbuf * om;

    /* Better be aligned */
    assert(((uint32_t)dptr & 3) == 0);

    block_len = rxpdu->om_omp->omp_databuf_len;
    rem_len = OS_MBUF_PKTHDR(rxpdu)->omp_len;
    src = dptr;

    /*
     * Setup for copying from first mbuf which is shorter due to packet header
     * and extra leading space
     */
    copy_len = block_len - rxpdu->om_pkthdr_len - 4;
    om = rxpdu;
    dst = om->om_data;

    while (om) {
        /*
         * Always copy blocks of length aligned to word size, only last mbuf
         * will have remaining non-word size bytes appended.
         */
        block_rem_len = copy_len;
        copy_len = min(copy_len, rem_len);
        copy_len &= ~3;

        dst = om->om_data;
        om->om_len = copy_len;
        rem_len -= copy_len;
        block_rem_len -= copy_len;

        __asm__ volatile (".syntax unified              \n"
                          "   mov  r4, %[len]           \n"
                          "   b    2f                   \n"
                          "1: ldr  r3, [%[src], %[len]] \n"
                          "   str  r3, [%[dst], %[len]] \n"
                          "2: subs %[len], #4           \n"
                          "   bpl  1b                   \n"
                          "   adds %[src], %[src], r4   \n"
                          "   adds %[dst], %[dst], r4   \n"
                          : [dst] "+l" (dst), [src] "+l" (src),
                            [len] "+l" (copy_len)
                          :
                          : "r3", "r4", "memory");

        if ((rem_len < 4) && (block_rem_len >= rem_len)) {
            break;
        }

        /* Move to next mbuf */
        om = SLIST_NEXT(om, om_next);
        copy_len = block_len;
    }

    /* Copy remaining bytes, if any, to last mbuf */
    om->om_len += rem_len;
    __asm__ volatile (".syntax unified              \n"
                      "   b    2f                   \n"
                      "1: ldrb r3, [%[src], %[len]] \n"
                      "   strb r3, [%[dst], %[len]] \n"
                      "2: subs %[len], #1           \n"
                      "   bpl  1b                   \n"
                      : [len] "+l" (rem_len)
                      : [dst] "l" (dst), [src] "l" (src)
                      : "r3", "memory");

    /* Copy header */
    memcpy(BLE_MBUF_HDR_PTR(rxpdu), &g_ble_phy_data.rxhdr,
           sizeof(struct ble_mbuf_hdr));
}

void
ble_phy_wfr_enable(int txrx, uint8_t tx_phy_mode, uint32_t wfr_usecs)
{
    uint64_t llt;
    uint32_t corr_window;
    uint32_t llt_z_ticks;
    uint32_t aa_time;

    /*
     * RX is started 2us earlier due to allowed clock accuracy and it should end
     * 2us later for the same reason. Preamble is always 8us (8 symbols on 1M,
     * 16 symbols on 2M) and Access Address is 32us on 1M and 16us on 2M. Add
     * 1us just in case...
     */
#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_2M_PHY)
    aa_time = 2 + (g_ble_phy_data.phy_mode_rx == BLE_PHY_MODE_1M ? 40 : 24) + 2 + 1;
#else
    aa_time = 45;
#endif

    if (txrx == BLE_PHY_WFR_ENABLE_TXRX) {
        CMAC_SETREGF(CM_PHY_CTRL2_REG, CORR_WINDOW, aa_time);
        CMAC->CM_EV_LINKUP_REG = CMAC_CM_EV_LINKUP_REG_LU_CORR_TMR_LD_2_CORR_START_Msk;
    } else if (wfr_usecs < 16384) {
        CMAC_SETREGF(CM_PHY_CTRL2_REG, CORR_WINDOW, wfr_usecs + aa_time);
        CMAC->CM_EV_LINKUP_REG = CMAC_CM_EV_LINKUP_REG_LU_CORR_TMR_LD_2_CORR_START_Msk;
    } else {
        wfr_usecs += aa_time;
        llt = g_ble_phy_data.start_llt;

        /*
         * wfr is outside range of CORR_WINDOW so we need to use LLT to start
         * correlator timeout with some delay. Let's use ~10ms as new CORR_WINDOW
         * value (does not really matter, just had to pick something) so need to
         * calculate how many hi-Z ticks of delay we need.
         */
        llt_z_ticks = (wfr_usecs - 10000) / 1024;

        /* New CORR_WINDOW is wfr adjusted by hi-Z ticks and remainder of 1st tick. */
        corr_window = wfr_usecs;
        corr_window -= llt_z_ticks * 1024;
        corr_window -= 1024 - (llt & 0x3ff);

        CMAC->CM_LL_TIMER1_36_10_EQ_Z_REG = (llt >> 10) + llt_z_ticks;
        CMAC_SETREGF(CM_PHY_CTRL2_REG, CORR_WINDOW, corr_window);
        CMAC->CM_EV_LINKUP_REG = CMAC_CM_EV_LINKUP_REG_LU_CORR_TMR_LD_2_TMR1_36_10_EQ_Z_Msk;
    }
}

int
ble_phy_init(void)
{
    g_ble_phy_data.phy_state = BLE_PHY_STATE_IDLE;
#if MYNEWT_VAL(BLE_LL_DTM)
    g_ble_phy_data.phy_whitening = 1;
#endif

    ble_rf_init();

    /*
     * 9_0_EQ_X can be linked to start RX/TX so we'll use this one for
     * scheduling TX/RX start - make sure it's not linked to LL_TIMER2LLC
     */
    CMAC->CM_LL_INT_SEL_REG &= ~CMAC_CM_LL_INT_SEL_REG_LL_TIMER1_9_0_EQ_X_SEL_Msk;

    CMAC->CM_PHY_CTRL_REG = ((PHY_DELAY_POWER_DN_RX - 1) << 24) |
                            ((PHY_DELAY_POWER_DN_TX - 1) << 16) |
                            ((PHY_DELAY_POWER_UP_RX - 1) << 8) |
                            ((PHY_DELAY_POWER_UP_TX - 1));

#if MYNEWT_VAL(BLE_PHY_RF_HP_MODE)
    CMAC->CM_PHY_CTRL2_REG = CMAC_CM_PHY_CTRL2_REG_PHY_MODE_Msk |
                             (3 << CMAC_CM_PHY_CTRL2_REG_CORR_CLK_MODE_Pos);
#else
    CMAC->CM_PHY_CTRL2_REG = CMAC_CM_PHY_CTRL2_REG_PHY_MODE_Msk |
                             (2 << CMAC_CM_PHY_CTRL2_REG_CORR_CLK_MODE_Pos);
#endif

    CMAC_SETREGF(CM_CTRL2_REG, WHITENING_MODE, 0);
    CMAC_SETREGF(CM_CTRL2_REG, CRC_MODE, 0);

    /* Setup for 1M by default */
    ble_phy_mode_set(BLE_PHY_MODE_1M, BLE_PHY_MODE_1M);
    ble_phy_mode_apply(BLE_PHY_MODE_1M);

    NVIC_SetPriority(FIELD_IRQn, 0);
    NVIC_SetPriority(CALLBACK_IRQn, 0);
    NVIC_SetPriority(FRAME_IRQn, 0);
    NVIC_SetPriority(CRYPTO_IRQn, 1);
    NVIC_SetPriority(SW_MAC_IRQn, 1);
    NVIC_EnableIRQ(FIELD_IRQn);
    NVIC_EnableIRQ(CALLBACK_IRQn);
    NVIC_EnableIRQ(FRAME_IRQn);
    NVIC_EnableIRQ(SW_MAC_IRQn);

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
    /* Initialize non-zero fixed values in CCM blocks */
    g_ble_phy_encrypt_data.b0[0] = 0x49;
    g_ble_phy_encrypt_data.b1[1] = 0x01;
    g_ble_phy_encrypt_data.ai[0] = 0x01;
#endif

    /*
     * Disable FIELD1, FIELD2 and FRAME errors since they can happen
     * sometimes if we are too late on scheduling and trigger CMAC
     * error. We can detect if tx_late happened and recover properly.
     */
    CMAC->CM_ERROR_DIS_REG |= CMAC_CM_ERROR_DIS_REG_CM_FIELD1_ERR_Msk |
                              CMAC_CM_ERROR_DIS_REG_CM_FIELD2_ERR_Msk |
                              CMAC_CM_ERROR_DIS_REG_CM_FRAME_ERR_Msk;

    return 0;
}

void
ble_phy_disable(void)
{
    MCU_DIAG_SER('D');

    __disable_irq();

    CMAC->CM_EV_SET_REG = CMAC_CM_EV_SET_REG_EV1C_BS_CLEAR_Msk;

    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    CMAC->CM_EXC_STAT_REG = CMAC_CM_EXC_STAT_REG_EXC_FIELD_ON_THR_EXP_Msk |
                            CMAC_CM_EXC_STAT_REG_EXC_BS_START_4THIS_Msk |
                            CMAC_CM_EXC_STAT_REG_EXC_CORR_TIMEOUT_Msk |
                            CMAC_CM_EXC_STAT_REG_EXC_BS_STOP_Msk |
                            CMAC_CM_EXC_STAT_REG_EXC_FRAME_START_Msk |
                            CMAC_CM_EXC_STAT_REG_EXC_PHY_TO_IDLE_4THIS_Msk |
                            CMAC_CM_EXC_STAT_REG_EXC_SW_MAC_Msk;

    NVIC->ICPR[0] = (1 << FIELD_IRQn) | (1 << CALLBACK_IRQn) |
                    (1 << FRAME_IRQn) | (1 << SW_MAC_IRQn);

    os_arch_cmac_bs_ctrl_irq_unblock();
    g_sw_mac_exc = 0;

    ble_rf_stop();

    /*
     * If ble_phy_disable is called precisely when access address was matched,
     * ts1_dirty may not be cleared properly. This is because bs_clear will
     * cause bitstream controller to be stopped and we won't get callback_irq,
     * but seems like demodulator is still active for a while and will trigger
     * ev1c_ts1_trigger on 1st symbol which will set ts1_dirty. We do not expect
     * ts1_dirty to be set after bs_clear so we won't clear it. To workaround
     * his, we can just clear it explicitly here after everything is already
     * disabled.
     */
    (void)CMAC->CM_TS1_REG;

    __enable_irq();

    g_ble_phy_data.phy_state = BLE_PHY_STATE_IDLE;
}

static void
ble_phy_rx_setup_fields(void)
{
    /* Make sure CRC LFSR initial value is set */
    CMAC_SETREGF(CM_CRC_REG, CRC_INIT_VAL, g_ble_phy_data.crc_init);

    CMAC->CM_FIELD_PUSH_DATA_REG = g_ble_phy_data.access_addr;
    CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_ACCESS_ADDR;
    CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_RX(0, 2);
    CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_HEADER;
#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
    if (g_ble_phy_data.phy_encrypted) {
        /* Only program one byte for encrypted payloads */
        CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_RX(2, 2);
        CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_ENC_PAYLOAD;
    } else {
        CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_RX(2, 2);
        CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_PAYLOAD;
    }
#else
    CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_RX(2, 2);
    CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_RX_PAYLOAD;
#endif
}

static void
ble_phy_rx_setup_xcvr(void)
{
    uint8_t rf_chan = g_ble_phy_chan_to_rf[g_ble_phy_data.channel];

    CMAC->CM_EV_SET_REG = CMAC_CM_EV_SET_REG_EV1C_CALLBACK_VALID_SET_Msk;

    ble_rf_setup_rx(rf_chan, g_ble_phy_data.phy_mode_rx);

    g_ble_phy_data.phy_rx_started = 0;
}

int
ble_phy_rx(void)
{
    MCU_DIAG_SER('R');

    ble_rf_configure();
    ble_phy_rx_setup_xcvr();

    CMAC->CM_FRAME_1_REG = CMAC_CM_FRAME_1_REG_FRAME_VALID_Msk;

    CMAC_SETREGF(CM_PHY_CTRL2_REG, CORR_WINDOW, 0);

    ble_phy_rx_setup_fields();

    g_ble_phy_data.phy_state = BLE_PHY_STATE_RX;

    return 0;
}

int
ble_phy_rx_set_start_time(uint32_t cputime, uint8_t rem_usecs)
{
    uint32_t ll_val32;
    int32_t time_till_start;

    MCU_DIAG_SER('r');

#if (BLE_LL_BT5_PHY_SUPPORTED == 1)
    ble_phy_mode_apply(g_ble_phy_data.phy_mode_rx);
#endif

    assert(ble_rf_is_enabled());

    ble_phy_rx();

    /* Get LL timer at cputime */
    ll_val32 = ble_phy_convert_and_record_start_time(cputime, rem_usecs);

    /* Add remaining usecs to get exact receive start time */
    ll_val32 += rem_usecs;

    /* Adjust start time for rx delays */
    ll_val32 -= PHY_DELAY_POWER_UP_RX - g_ble_phy_data.path_delay_rx;

    __disable_irq();
    CMAC->CM_LL_TIMER1_9_0_EQ_X_REG = ll_val32;
    CMAC->CM_EV_LINKUP_REG =
        CMAC_CM_EV_LINKUP_REG_LU_FRAME_START_2_TMR1_9_0_EQ_X_Msk;

    time_till_start = (int32_t)(ll_val32 - cmac_timer_read32());
    if (time_till_start <= 0) {
        /*
         * Possible we missed the frame start! If we have, we need to start
         * ASAP.
         */
        if ((CMAC->CM_EXC_STAT_REG & CMAC_CM_EXC_STAT_REG_EXC_FRAME_START_Msk) == 0) {
            /* We missed start. Start now */
            CMAC->CM_EV_LINKUP_REG =
                CMAC_CM_EV_LINKUP_REG_LU_FRAME_START_2_ASAP_Msk;
        }
    }
    __enable_irq();

    return 0;
}

int
ble_phy_tx(ble_phy_tx_pducb_t pducb, void *pducb_arg, uint8_t end_trans)
{
    uint8_t *txbuf = g_ble_phy_tx_buf;
    int rc;

    MCU_DIAG_SER('T');

    assert(CMAC->CM_FRAME_1_REG & CMAC_CM_FRAME_1_REG_FRAME_TX_Msk);

    g_ble_phy_data.end_transition = end_trans;

    /*
     * Program required fields now so in worst case TX can continue while we
     * are still preparing header and payload.
     */
    CMAC->CM_FIELD_PUSH_DATA_REG = g_ble_phy_data.access_addr & 1 ? 0x5555 : 0xaaaa;
    CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_TX_PREAMBLE;
    CMAC->CM_FIELD_PUSH_DATA_REG = g_ble_phy_data.access_addr;
    CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_TX_ACCESS_ADDR;

    /* Make sure CRC LFSR initial value is set */
    CMAC_SETREGF(CM_CRC_REG, CRC_INIT_VAL, g_ble_phy_data.crc_init);

    /* txbuf[0] is hdr_byte, txbuf[1] is pkt_len */
    txbuf[1] = pducb(&txbuf[2], pducb_arg, &txbuf[0]);

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
    if (g_ble_phy_data.phy_encrypted && (txbuf[1] != 0)) {
        /* We have to add the MIC to the length */
        txbuf[1] += BLE_LL_DATA_MIC_LEN;

        /* Program header field */
        CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_TX(0, 2);
        CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_TX_PAYLOAD;

        /* Program payload (and MIC) */
        CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_TX(2, txbuf[1]);
        CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_TX_ENC_PAYLOAD;
    } else {
        /* Program header and payload fields */
        CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_TX(0, txbuf[1] + 2);
        CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_TX_PAYLOAD;
    }
#else
    /* Program header and payload fields */
    CMAC->CM_FIELD_PUSH_DATA_REG = FIELD_DATA_REG_DMA_TX(0, txbuf[1] + 2);
    CMAC->CM_FIELD_PUSH_CTRL_REG = FIELD_CTRL_REG_TX_PAYLOAD;
#endif

    /*
     * If there was FIELD_ON_THR exception it means access address was already
     * sent and we are likely too late here - abort.
     */
    if (CMAC->CM_EXC_STAT_REG & CMAC_CM_EXC_STAT_REG_EXC_FIELD_ON_THR_EXP_Msk) {
        ble_phy_disable();
        g_ble_phy_data.end_transition = BLE_PHY_TRANSITION_NONE;
        STATS_INC(ble_phy_stats, tx_late_field);
        STATS_INC(ble_phy_stats, tx_late);
        rc = BLE_PHY_ERR_RADIO_STATE;
    } else {
        if (g_ble_phy_data.phy_state == BLE_PHY_STATE_IDLE) {
            g_ble_phy_data.phy_state = BLE_PHY_STATE_TX;
        }
        STATS_INC(ble_phy_stats, tx_good);
        STATS_INCN(ble_phy_stats, tx_bytes, txbuf[1] + 2);
        rc = BLE_ERR_SUCCESS;
    }

    /* Now we can handle BS_CTRL */
    NVIC_EnableIRQ(FRAME_IRQn);
    NVIC_EnableIRQ(FIELD_IRQn);

    return rc;
}

int
ble_phy_tx_set_start_time(uint32_t cputime, uint8_t rem_usecs)
{
    uint8_t rf_chan = g_ble_phy_chan_to_rf[g_ble_phy_data.channel];
    uint32_t ll_val32;
    int rc;

    MCU_DIAG_SER('t');

#if (BLE_LL_BT5_PHY_SUPPORTED == 1)
    ble_phy_mode_apply(g_ble_phy_data.phy_mode_tx);
#endif

    assert(ble_rf_is_enabled());

    ble_rf_configure();
    ble_rf_setup_tx(rf_chan, g_ble_phy_data.phy_mode_tx);

    ll_val32 = ble_phy_convert_and_record_start_time(cputime, rem_usecs);
    ll_val32 += rem_usecs;
    ll_val32 -= PHY_DELAY_POWER_UP_TX + g_ble_phy_data.path_delay_tx;
    /* we can schedule TX only up to 1023us in advance */
    assert((int32_t)(ll_val32 - cmac_timer_read32()) < 1024);

    /*
     * We do not want FIELD/FRAME interrupts until ble_phy_tx() has
     * pushed all fields.
     */
    NVIC_DisableIRQ(FRAME_IRQn);
    NVIC_DisableIRQ(FIELD_IRQn);

    CMAC->CM_LL_TIMER1_9_0_EQ_X_REG = ll_val32;
    CMAC->CM_EV_LINKUP_REG = CMAC_CM_EV_LINKUP_REG_LU_FRAME_START_2_TMR1_9_0_EQ_X_Msk;

    if ((int32_t)(ll_val32 - cmac_timer_read32()) < 0) {
        STATS_INC(ble_phy_stats, tx_late_sched);
        goto tx_late;
    }

    /*
     * Program frame now since it needs to be ready for FRAME_START, we can
     * push fields later
     */
    CMAC->CM_FRAME_1_REG = CMAC_CM_FRAME_1_REG_FRAME_VALID_Msk |
                           CMAC_CM_FRAME_1_REG_FRAME_TX_Msk |
                           CMAC_CM_FRAME_1_REG_FRAME_EXC_ON_BS_START_Msk;

    /*
     * There should be no EXC_FRAME_START here so if it already happened we
     * need to assume tx_late and abort.
     */
    if (CMAC->CM_EXC_STAT_REG & CMAC_CM_EXC_STAT_REG_EXC_FRAME_START_Msk) {
        STATS_INC(ble_phy_stats, tx_late_frame);
        goto tx_late;
    }

    rc = 0;

    goto done;

tx_late:
    STATS_INC(ble_phy_stats, tx_late);
    ble_phy_disable();
    NVIC_EnableIRQ(FRAME_IRQn);
    NVIC_EnableIRQ(FIELD_IRQn);
    rc = BLE_PHY_ERR_TX_LATE;

done:
    return rc;
}

void
ble_phy_set_txend_cb(ble_phy_tx_end_func txend_cb, void *arg)
{
    g_ble_phy_data.txend_cb = txend_cb;
    g_ble_phy_data.txend_arg = arg;
}

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
void
ble_phy_tx_enc_start(void)
{
    struct ble_phy_encrypt_obj *enc;

    enc = &g_ble_phy_encrypt_data;
    enc->b0[15] = g_ble_phy_tx_buf[1] - 4;
    enc->b1[2] = g_ble_phy_tx_buf[0] & BLE_LL_DATA_HDR_LLID_MASK;

    /* XXX: should we check for busy? */
    /* XXX: might not be needed, but for now terminate any crypto operations. */
    CMAC->CM_CRYPTO_CTRL_REG = CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_SW_REQ_ABORT_Msk;

    CMAC->CM_CRYPTO_CTRL_REG = CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_IN_SEL_Msk |
                               CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_CTR_MAC_EN_Msk |
                               CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_CTR_PLD_EN_Msk |
                               CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_AUTH_EN_Msk |
                               CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_ENC_DECN_Msk;

    /* Start crypto */
    CMAC->CM_EV_SET_REG = CMAC_CM_EV_SET_REG_EV_CRYPTO_START_Msk;
}

void
ble_phy_rx_enc_start(uint8_t len)
{
    struct ble_phy_encrypt_obj *enc;

    enc = &g_ble_phy_encrypt_data;
    enc->b0[15] = len - 4; /* length without MIC as length includes MIC */
    enc->b1[2] = g_ble_phy_rx_buf[0] & BLE_LL_DATA_HDR_LLID_MASK;

    /* XXX: should we check for busy? */
    /* XXX: might not be needed, but for now terminate any crypto operations. */
    //CMAC->CM_CRYPTO_CTRL_REG = CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_SW_REQ_ABORT_Msk;

    CMAC->CM_CRYPTO_CTRL_REG = CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_OUT_SEL_Msk |
        CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_CTR_MAC_EN_Msk |
        CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_CTR_PLD_EN_Msk |
        CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_AUTH_EN_Msk;

    /* Start crypto */
    CMAC->CM_EV_SET_REG = CMAC_CM_EV_SET_REG_EV_CRYPTO_START_Msk;
}

void
ble_phy_encrypt_enable(uint64_t pkt_counter, uint8_t *iv, uint8_t *key,
                       uint8_t is_master)
{
    struct ble_phy_encrypt_obj *enc;

    enc = &g_ble_phy_encrypt_data;
    memcpy(enc->key, key, 16);
    memcpy(&enc->b0[6], iv, 8);
    put_le32(&enc->b0[1], pkt_counter);
    enc->b0[5] = is_master ? 0x80 : 0;
    memcpy(&enc->ai[6], iv, 8);
    put_le32(&enc->ai[1], pkt_counter);
    enc->ai[5] = enc->b0[5];

    g_ble_phy_data.phy_encrypted = 1;

    /* Program key registers */
    CMAC->CM_CRYPTO_KEY_31_0_REG = get_le32(&enc->key[0]);
    CMAC->CM_CRYPTO_KEY_63_32_REG = get_le32(&enc->key[4]);
    CMAC->CM_CRYPTO_KEY_95_64_REG = get_le32(&enc->key[8]);
    CMAC->CM_CRYPTO_KEY_127_96_REG = get_le32(&enc->key[12]);

    /* Program ADRx registers */
    CMAC->CM_CRYPTO_IN_ADR0_REG = (uint32_t)enc->b1;
    CMAC->CM_CRYPTO_IN_ADR1_REG = (uint32_t)enc->b0;
    CMAC->CM_CRYPTO_IN_ADR2_REG = (uint32_t)&g_ble_phy_tx_buf[2];
    CMAC->CM_CRYPTO_IN_ADR3_REG = (uint32_t)enc->ai;
    CMAC->CM_CRYPTO_OUT_ADR_REG = (uint32_t)&g_ble_phy_rx_buf[2];

    CMAC->CM_CRYPTO_CTRL_REG = CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_SW_REQ_PBUF_CLR_Msk;
}

void
ble_phy_encrypt_set_pkt_cntr(uint64_t pkt_counter, int dir)
{
    struct ble_phy_encrypt_obj *enc;

    enc = &g_ble_phy_encrypt_data;
    put_le32(&enc->b0[1], pkt_counter);
    enc->b0[5] = dir ? 0x80 : 0;
    put_le32(&enc->ai[1], pkt_counter);
    enc->ai[5] = enc->b0[5];

    CMAC->CM_CRYPTO_CTRL_REG = CMAC_CM_CRYPTO_CTRL_REG_CM_CRYPTO_SW_REQ_PBUF_CLR_Msk;
}

void
ble_phy_encrypt_disable(void)
{
    g_ble_phy_data.phy_encrypted = 0;
}
#endif

int
ble_phy_txpwr_set(int dbm)
{
#if MYNEWT_VAL(CMAC_DEBUG_DATA_ENABLE)
    if (g_cmac_shared_data.debug.tx_power_override != INT8_MAX) {
        ble_rf_set_tx_power(g_cmac_shared_data.debug.tx_power_override);
    } else {
        ble_rf_set_tx_power(dbm);
    }
#else
    ble_rf_set_tx_power(dbm);
#endif

    return 0;
}

int
ble_phy_txpower_round(int dbm)
{
    return 0;
}

void
ble_phy_set_rx_pwr_compensation(int8_t compensation)
{
}

int
ble_phy_setchan(uint8_t chan, uint32_t access_addr, uint32_t crc_init)
{
    uint8_t rf_chan = g_ble_phy_chan_to_rf[chan];

    assert(chan < BLE_PHY_NUM_CHANS);

    if (chan >= BLE_PHY_NUM_CHANS) {
        return BLE_PHY_ERR_INV_PARAM;
    }

    g_ble_phy_data.channel = chan;
    g_ble_phy_data.access_addr = access_addr;
    g_ble_phy_data.crc_init = crc_init;

    CMAC_SETREGF(CM_PHY_CTRL2_REG, PHY_RF_CHANNEL, rf_chan);

    return 0;
}

void
ble_phy_restart_rx(void)
{
    /* XXX: for now, we will disable the phy using ble_phy_disable and then
       re-enable it
     */
    ble_phy_disable();

    /* Apply mode before starting RX */
#if (BLE_LL_BT5_PHY_SUPPORTED == 1)
    ble_phy_mode_apply(g_ble_phy_data.phy_mode_rx);
#endif

    /* Setup phy to rx again */
    ble_phy_rx();

    /* Start reception now */
    CMAC->CM_EV_LINKUP_REG = CMAC_CM_EV_LINKUP_REG_LU_FRAME_START_2_ASAP_Msk;
}

uint32_t
ble_phy_access_addr_get(void)
{
    return g_ble_phy_data.access_addr;
}

int
ble_phy_state_get(void)
{
    return g_ble_phy_data.phy_state;
}

int
ble_phy_rx_started(void)
{
    return g_ble_phy_data.phy_rx_started;
}

uint8_t
ble_phy_xcvr_state_get(void)
{
    return ble_rf_is_enabled();
}

uint8_t
ble_phy_max_data_pdu_pyld(void)
{
    return BLE_LL_DATA_PDU_MAX_PYLD;
}

void
ble_phy_resolv_list_enable(void)
{
    g_ble_phy_data.phy_privacy = 1;

    ble_hw_resolv_proc_enable();
}

void
ble_phy_resolv_list_disable(void)
{
    ble_hw_resolv_proc_disable();

    g_ble_phy_data.phy_privacy = 0;
}

void
ble_phy_rfclk_enable(void)
{
    ble_rf_enable();
}

void
ble_phy_rfclk_disable(void)
{
    /* XXX We can't disable RF while PHY_BUSY is asserted so let's wait a bit */
    while (CMAC->CM_DIAG_WORD2_REG & CMAC_CM_DIAG_WORD2_REG_DIAG2_PHY_BUSY_BUF_Msk);

    ble_rf_disable();
}

#if MYNEWT_VAL(BLE_LL_DTM)
void
ble_phy_enable_dtm(void)
{
    g_ble_phy_data.phy_whitening = 0;
}

void
ble_phy_disable_dtm(void)
{
    g_ble_phy_data.phy_whitening = 1;
}
#endif
