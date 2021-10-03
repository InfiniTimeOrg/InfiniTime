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

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <syscfg/syscfg.h>
#include <os/os.h>
#include <nimble/ble.h>
#include <nimble/nimble_opt.h>
#include <nimble/nimble_npl.h>
#include <controller/ble_phy.h>

#include <ble/xcvr.h>
#include <controller/ble_phy_trace.h>
#include <controller/ble_ll.h>
#include <mcu/nrf5340_net_clock.h>
#include <mcu/cmsis_nvic.h>

/*
 * NOTE: This code uses 0-5 DPPI channels so care should be taken when using
 * DPPI somewhere else.
 * TODO maybe we could reduce number of used channels if we reuse same channel
 * for mutually exclusive events but for now make it simpler to debug.
 */

#define DPPI_CH_TIMER0_EVENTS_COMPARE_0         0
#define DPPI_CH_TIMER0_EVENTS_COMPARE_3         1
#define DPPI_CH_RADIO_EVENTS_END                2
#define DPPI_CH_RADIO_EVENTS_BCMATCH            3
#define DPPI_CH_RADIO_EVENTS_ADDRESS            4
#define DPPI_CH_RTC0_EVENTS_COMPARE_0           5

#define DPPI_CH_ENABLE_ALL (DPPIC_CHEN_CH0_Msk | DPPIC_CHEN_CH1_Msk | DPPIC_CHEN_CH2_Msk | \
                            DPPIC_CHEN_CH3_Msk |  DPPIC_CHEN_CH4_Msk | DPPIC_CHEN_CH5_Msk)

#define DPPI_PUBLISH_TIMER0_EVENTS_COMPARE_0    ((DPPI_CH_TIMER0_EVENTS_COMPARE_0 << TIMER_PUBLISH_COMPARE_CHIDX_Pos) | \
                                                 (TIMER_PUBLISH_COMPARE_EN_Enabled << TIMER_PUBLISH_COMPARE_EN_Pos))
#define DPPI_PUBLISH_TIMER0_EVENTS_COMPARE_3    ((DPPI_CH_TIMER0_EVENTS_COMPARE_3 << TIMER_PUBLISH_COMPARE_CHIDX_Pos) | \
                                                 (TIMER_PUBLISH_COMPARE_EN_Enabled << TIMER_PUBLISH_COMPARE_EN_Pos))
#define DPPI_PUBLISH_RADIO_EVENTS_END           ((DPPI_CH_RADIO_EVENTS_END << RADIO_PUBLISH_END_CHIDX_Pos) | \
                                                 (RADIO_PUBLISH_END_EN_Enabled << RADIO_PUBLISH_END_EN_Pos))
#define DPPI_PUBLISH_RADIO_EVENTS_BCMATCH       ((DPPI_CH_RADIO_EVENTS_BCMATCH << RADIO_PUBLISH_BCMATCH_CHIDX_Pos) | \
                                                 (RADIO_PUBLISH_BCMATCH_EN_Enabled << RADIO_PUBLISH_BCMATCH_EN_Pos))
#define DPPI_PUBLISH_RADIO_EVENTS_ADDRESS       ((DPPI_CH_RADIO_EVENTS_ADDRESS << RADIO_PUBLISH_ADDRESS_CHIDX_Pos) | \
                                                 (RADIO_PUBLISH_ADDRESS_EN_Enabled << RADIO_PUBLISH_ADDRESS_EN_Pos))
#define DPPI_PUBLISH_RTC0_EVENTS_COMPARE_0      ((DPPI_CH_RTC0_EVENTS_COMPARE_0 << RTC_PUBLISH_COMPARE_CHIDX_Pos) | \
                                                 (RTC_PUBLISH_COMPARE_EN_Enabled << RTC_PUBLISH_COMPARE_EN_Pos))

#define DPPI_SUBSCRIBE_TIMER0_TASKS_START(_enable)     ((DPPI_CH_RTC0_EVENTS_COMPARE_0 << TIMER_SUBSCRIBE_START_CHIDX_Pos) | \
                                                        ((_enable) << TIMER_SUBSCRIBE_START_EN_Pos))
#define DPPI_SUBSCRIBE_TIMER0_TASKS_CAPTURE1(_enable)  ((DPPI_CH_RADIO_EVENTS_ADDRESS << TIMER_SUBSCRIBE_CAPTURE_CHIDX_Pos) | \
                                                        ((_enable) << TIMER_SUBSCRIBE_CAPTURE_EN_Pos))
#define DPPI_SUBSCRIBE_TIMER0_TASKS_CAPTURE2(_enable)  ((DPPI_CH_RADIO_EVENTS_END << TIMER_SUBSCRIBE_CAPTURE_CHIDX_Pos) | \
                                                        ((_enable) << TIMER_SUBSCRIBE_CAPTURE_EN_Pos))
#define DPPI_SUBSCRIBE_TIMER0_TASKS_CAPTURE3(_enable)  ((DPPI_CH_RADIO_EVENTS_ADDRESS << TIMER_SUBSCRIBE_CAPTURE_CHIDX_Pos) | \
                                                        ((_enable) << TIMER_SUBSCRIBE_CAPTURE_EN_Pos))
#define DPPI_SUBSCRIBE_RADIO_TASKS_DISABLE(_enable)    ((DPPI_CH_TIMER0_EVENTS_COMPARE_3 << RADIO_SUBSCRIBE_DISABLE_CHIDX_Pos) | \
                                                        ((_enable) << RADIO_SUBSCRIBE_DISABLE_EN_Pos))
#define DPPI_SUBSCRIBE_RADIO_TASKS_RXEN(_enable)       ((DPPI_CH_TIMER0_EVENTS_COMPARE_0 << RADIO_SUBSCRIBE_RXEN_CHIDX_Pos) | \
                                                        ((_enable) << RADIO_SUBSCRIBE_RXEN_EN_Pos))
#define DPPI_SUBSCRIBE_RADIO_TASKS_TXEN(_enable)       ((DPPI_CH_TIMER0_EVENTS_COMPARE_0 << RADIO_SUBSCRIBE_TXEN_CHIDX_Pos) | \
                                                        ((_enable) << RADIO_SUBSCRIBE_TXEN_EN_Pos))
#define DPPI_SUBSCRIBE_AAR_TASKS_START(_enable)        ((DPPI_CH_RADIO_EVENTS_BCMATCH << AAR_SUBSCRIBE_START_CHIDX_Pos) | \
                                                        ((_enable) << AAR_SUBSCRIBE_START_EN_Pos))
#define DPPI_SUBSCRIBE_CCM_TASKS_CRYPT(_enable)        ((DPPI_CH_RADIO_EVENTS_ADDRESS << CCM_SUBSCRIBE_CRYPT_CHIDX_Pos) | \
                                                        ((_enable) << CCM_SUBSCRIBE_CRYPT_EN_Pos))

extern uint8_t g_nrf_num_irks;
extern uint32_t g_nrf_irk_list[];

/* To disable all radio interrupts */
#define NRF_RADIO_IRQ_MASK_ALL (0x34FF)

/*
 * We configure the nrf with a 1 byte S0 field, 8 bit length field, and
 * zero bit S1 field. The preamble is 8 bits long.
 */
#define NRF_LFLEN_BITS      (8)
#define NRF_S0LEN           (1)
#define NRF_S1LEN_BITS      (0)
#define NRF_CILEN_BITS      (2)
#define NRF_TERMLEN_BITS    (3)

/* Maximum length of frames */
#define NRF_MAXLEN          (255)
#define NRF_BALEN           (3)     /* For base address of 3 bytes */

/* NRF_RADIO_NS->PCNF0 configuration values */
#define NRF_PCNF0           (NRF_LFLEN_BITS << RADIO_PCNF0_LFLEN_Pos) | \
                            (RADIO_PCNF0_S1INCL_Msk) | \
                            (NRF_S0LEN << RADIO_PCNF0_S0LEN_Pos) | \
                            (NRF_S1LEN_BITS << RADIO_PCNF0_S1LEN_Pos)
#define NRF_PCNF0_1M        (NRF_PCNF0) | \
                            (RADIO_PCNF0_PLEN_8bit << RADIO_PCNF0_PLEN_Pos)
#define NRF_PCNF0_2M        (NRF_PCNF0) | \
                            (RADIO_PCNF0_PLEN_16bit << RADIO_PCNF0_PLEN_Pos)
#define NRF_PCNF0_CODED     (NRF_PCNF0) | \
                            (RADIO_PCNF0_PLEN_LongRange << RADIO_PCNF0_PLEN_Pos) | \
                            (NRF_CILEN_BITS << RADIO_PCNF0_CILEN_Pos) | \
                            (NRF_TERMLEN_BITS << RADIO_PCNF0_TERMLEN_Pos)

/* BLE PHY data structure */
struct ble_phy_obj {
    uint8_t phy_stats_initialized;
    int8_t phy_txpwr_dbm;
    uint8_t phy_chan;
    uint8_t phy_state;
    uint8_t phy_transition;
    uint8_t phy_transition_late;
    uint8_t phy_rx_started;
    uint8_t phy_encrypted;
    uint8_t phy_privacy;
    uint8_t phy_tx_pyld_len;
    uint8_t phy_cur_phy_mode;
    uint8_t phy_tx_phy_mode;
    uint8_t phy_rx_phy_mode;
    uint8_t phy_bcc_offset;
    int8_t rx_pwr_compensation;
    uint32_t phy_aar_scratch;
    uint32_t phy_access_address;
    struct ble_mbuf_hdr rxhdr;
    void *txend_arg;
    ble_phy_tx_end_func txend_cb;
    uint32_t phy_start_cputime;
};
struct ble_phy_obj g_ble_phy_data;

/* XXX: if 27 byte packets desired we can make this smaller */
/* Global transmit/receive buffer */
static uint32_t g_ble_phy_tx_buf[(BLE_PHY_MAX_PDU_LEN + 3) / 4];
static uint32_t g_ble_phy_rx_buf[(BLE_PHY_MAX_PDU_LEN + 3) / 4];

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
/* Make sure word-aligned for faster copies */
static uint32_t g_ble_phy_enc_buf[(BLE_PHY_MAX_PDU_LEN + 3) / 4];
#endif

/* RF center frequency for each channel index (offset from 2400 MHz) */
static const uint8_t g_ble_phy_chan_freq[BLE_PHY_NUM_CHANS] = {
    4,  6,  8, 10, 12, 14, 16, 18, 20, 22, /* 0-9 */
    24, 28, 30, 32, 34, 36, 38, 40, 42, 44, /* 10-19 */
    46, 48, 50, 52, 54, 56, 58, 60, 62, 64, /* 20-29 */
    66, 68, 70, 72, 74, 76, 78,  2, 26, 80, /* 30-39 */
};

#if (BLE_LL_BT5_PHY_SUPPORTED == 1)
/* packet start offsets (in usecs) */
static const uint16_t g_ble_phy_mode_pkt_start_off[BLE_PHY_NUM_MODE] = {
    [BLE_PHY_MODE_1M] = 40,
    [BLE_PHY_MODE_2M] = 24,
    [BLE_PHY_MODE_CODED_125KBPS] = 376,
    [BLE_PHY_MODE_CODED_500KBPS] = 376
};
#endif

/* Various radio timings */
/* Radio ramp-up times in usecs (fast mode) */
#define BLE_PHY_T_TXENFAST      (XCVR_TX_RADIO_RAMPUP_USECS)
#define BLE_PHY_T_RXENFAST      (XCVR_RX_RADIO_RAMPUP_USECS)

/* delay between EVENTS_READY and start of tx */
static const uint8_t g_ble_phy_t_txdelay[BLE_PHY_NUM_MODE] = {
    [BLE_PHY_MODE_1M] = 4,
    [BLE_PHY_MODE_2M] = 3,
    [BLE_PHY_MODE_CODED_125KBPS] = 5,
    [BLE_PHY_MODE_CODED_500KBPS] = 5
};
/* delay between EVENTS_END and end of txd packet */
static const uint8_t g_ble_phy_t_txenddelay[BLE_PHY_NUM_MODE] = {
    [BLE_PHY_MODE_1M] = 4,
    [BLE_PHY_MODE_2M] = 3,
    [BLE_PHY_MODE_CODED_125KBPS] = 9,
    [BLE_PHY_MODE_CODED_500KBPS] = 3
};
/* delay between rxd access address (w/ TERM1 for coded) and EVENTS_ADDRESS */
static const uint8_t g_ble_phy_t_rxaddrdelay[BLE_PHY_NUM_MODE] = {
    [BLE_PHY_MODE_1M] = 6,
    [BLE_PHY_MODE_2M] = 2,
    [BLE_PHY_MODE_CODED_125KBPS] = 17,
    [BLE_PHY_MODE_CODED_500KBPS] = 17
};
/* delay between end of rxd packet and EVENTS_END */
static const uint8_t g_ble_phy_t_rxenddelay[BLE_PHY_NUM_MODE] = {
    [BLE_PHY_MODE_1M] = 6,
    [BLE_PHY_MODE_2M] = 2,
    [BLE_PHY_MODE_CODED_125KBPS] = 27,
    [BLE_PHY_MODE_CODED_500KBPS] = 22
};

/* Statistics */
STATS_SECT_START(ble_phy_stats)
STATS_SECT_ENTRY(phy_isrs)
STATS_SECT_ENTRY(tx_good)
STATS_SECT_ENTRY(tx_fail)
STATS_SECT_ENTRY(tx_late)
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

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
/*
 * Per nordic, the number of bytes needed for scratch is 16 + MAX_PKT_SIZE.
 * However, when I used a smaller size it still overwrote the scratchpad. Until
 * I figure this out I am just going to allocate 67 words so we have enough
 * space for 267 bytes of scratch. I used 268 bytes since not sure if this
 * needs to be aligned and burning a byte is no big deal.
 *
 *#define NRF_ENC_SCRATCH_WORDS (((MYNEWT_VAL(BLE_LL_MAX_PKT_SIZE) + 16) + 3) / 4)
 */
#define NRF_ENC_SCRATCH_WORDS   (67)

static uint32_t nrf_encrypt_scratchpad[NRF_ENC_SCRATCH_WORDS];

struct nrf_ccm_data {
    uint8_t key[16];
    uint64_t pkt_counter;
    uint8_t dir_bit;
    uint8_t iv[8];
} __attribute__((packed));

static struct nrf_ccm_data nrf_ccm_data;
#endif

#if (BLE_LL_BT5_PHY_SUPPORTED == 1)

uint32_t
ble_phy_mode_pdu_start_off(int phy_mode)
{
    return g_ble_phy_mode_pkt_start_off[phy_mode];
}

static void
ble_phy_mode_apply(uint8_t phy_mode)
{
    if (phy_mode == g_ble_phy_data.phy_cur_phy_mode) {
        return;
    }

    switch (phy_mode) {
    case BLE_PHY_MODE_1M:
        NRF_RADIO_NS->MODE = RADIO_MODE_MODE_Ble_1Mbit;
        NRF_RADIO_NS->PCNF0 = NRF_PCNF0_1M;
        break;
#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_2M_PHY)
    case BLE_PHY_MODE_2M:
        NRF_RADIO_NS->MODE = RADIO_MODE_MODE_Ble_2Mbit;
        NRF_RADIO_NS->PCNF0 = NRF_PCNF0_2M;
        break;
#endif
#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_CODED_PHY)
    case BLE_PHY_MODE_CODED_125KBPS:
        NRF_RADIO_NS->MODE = RADIO_MODE_MODE_Ble_LR125Kbit;
        NRF_RADIO_NS->PCNF0 = NRF_PCNF0_CODED;
        break;
    case BLE_PHY_MODE_CODED_500KBPS:
        NRF_RADIO_NS->MODE = RADIO_MODE_MODE_Ble_LR500Kbit;
        NRF_RADIO_NS->PCNF0 = NRF_PCNF0_CODED;
        break;
#endif
    default:
        assert(0);
    }

    g_ble_phy_data.phy_cur_phy_mode = phy_mode;
}
#endif

void
ble_phy_mode_set(uint8_t tx_phy_mode, uint8_t rx_phy_mode)
{
    g_ble_phy_data.phy_tx_phy_mode = tx_phy_mode;
    g_ble_phy_data.phy_rx_phy_mode = rx_phy_mode;
}

int
ble_phy_get_cur_phy(void)
{
#if (BLE_LL_BT5_PHY_SUPPORTED == 1)
    switch (g_ble_phy_data.phy_cur_phy_mode) {
    case BLE_PHY_MODE_1M:
        return BLE_PHY_1M;
    case BLE_PHY_MODE_2M:
        return BLE_PHY_2M;
    case BLE_PHY_MODE_CODED_125KBPS:
    case BLE_PHY_MODE_CODED_500KBPS:
        return BLE_PHY_CODED;
    default:
        assert(0);
        return -1;
    }
#else
    return BLE_PHY_1M;
#endif
}

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

    while (true) {
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
                          : [dst] "+r" (dst), [src] "+r" (src),
                          [len] "+r" (copy_len)
                          :
                          : "r3", "r4", "memory"
                          );

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
                      : [len] "+r" (rem_len)
                      : [dst] "r" (dst), [src] "r" (src)
                      : "r3", "memory"
                      );

    /* Copy header */
    memcpy(BLE_MBUF_HDR_PTR(rxpdu), &g_ble_phy_data.rxhdr,
           sizeof(struct ble_mbuf_hdr));
}

/**
 * Called when we want to wait if the radio is in either the rx or tx
 * disable states. We want to wait until that state is over before doing
 * anything to the radio
 */
static void
nrf_wait_disabled(void)
{
    uint32_t state;

    state = NRF_RADIO_NS->STATE;
    if (state != RADIO_STATE_STATE_Disabled) {
        if ((state == RADIO_STATE_STATE_RxDisable) ||
            (state == RADIO_STATE_STATE_TxDisable)) {
            /* This will end within a short time (6 usecs). Just poll */
            while (NRF_RADIO_NS->STATE == state) {
                /* If this fails, something is really wrong. Should last
                 * no more than 6 usecs
                 */
            }
        }
    }
}

static int
ble_phy_set_start_time(uint32_t cputime, uint8_t rem_usecs, bool tx)
{
    uint32_t next_cc;
    uint32_t cur_cc;
    uint32_t cntr;
    uint32_t delta;

    /*
     * We need to adjust start time to include radio ramp-up and TX pipeline
     * delay (the latter only if applicable, so only for TX).
     *
     * Radio ramp-up time is 40 usecs and TX delay is 3 or 5 usecs depending on
     * phy, thus we'll offset RTC by 2 full ticks (61 usecs) and then compensate
     * using TIMER0 with 1 usec precision.
     */

    cputime -= 2;
    rem_usecs += 61;
    if (tx) {
        rem_usecs -= BLE_PHY_T_TXENFAST;
        rem_usecs -= g_ble_phy_t_txdelay[g_ble_phy_data.phy_cur_phy_mode];
    } else {
        rem_usecs -= BLE_PHY_T_RXENFAST;
    }

    /*
     * rem_usecs will be no more than 2 ticks, but if it is more than single
     * tick then we should better count one more low-power tick rather than
     * 30 high-power usecs. Also make sure we don't set TIMER0 CC to 0 as the
     * compare won't occur.
     */

    if (rem_usecs > 30) {
        cputime++;
        rem_usecs -= 30;
    }

    /*
     * Can we set the RTC compare to start TIMER0? We can do it if:
     *      a) Current compare value is not N+1 or N+2 ticks from current
     *      counter.
     *      b) The value we want to set is not at least N+2 from current
     *      counter.
     *
     * NOTE: since the counter can tick 1 while we do these calculations we
     * need to account for it.
     */
    next_cc = cputime & 0xffffff;
    cur_cc = NRF_RTC0_NS->CC[0];
    cntr = NRF_RTC0_NS->COUNTER;

    delta = (cur_cc - cntr) & 0xffffff;
    if ((delta <= 3) && (delta != 0)) {
        return -1;
    }
    delta = (next_cc - cntr) & 0xffffff;
    if ((delta & 0x800000) || (delta < 3)) {
        return -1;
    }

    /* Clear and set TIMER0 to fire off at proper time */
    NRF_TIMER0_NS->TASKS_CLEAR = 1;
    NRF_TIMER0_NS->CC[0] = rem_usecs;
    NRF_TIMER0_NS->EVENTS_COMPARE[0] = 0;

    /* Set RTC compare to start TIMER0 */
    NRF_RTC0_NS->EVENTS_COMPARE[0] = 0;
    NRF_RTC0_NS->CC[0] = next_cc;
    NRF_RTC0_NS->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;

    /* Enable PPI */
    NRF_TIMER0_NS->SUBSCRIBE_START = DPPI_SUBSCRIBE_TIMER0_TASKS_START(1);

    /* Store the cputime at which we set the RTC */
    g_ble_phy_data.phy_start_cputime = cputime;

    return 0;
}

static int
ble_phy_set_start_now(void)
{
    os_sr_t sr;
    uint32_t now;

    OS_ENTER_CRITICAL(sr);

    /*
     * Set TIMER0 to fire immediately. We can't set CC to 0 as compare will not
     * occur in such case.
     */
    NRF_TIMER0_NS->TASKS_CLEAR = 1;
    NRF_TIMER0_NS->CC[0] = 1;
    NRF_TIMER0_NS->EVENTS_COMPARE[0] = 0;

    /*
     * Set RTC compare to start TIMER0. We need to set it to at least N+2 ticks
     * from current value to guarantee triggering compare event, but let's set
     * it to N+3 to account for possible extra tick on RTC0 during these
     * operations.
     */
    now = os_cputime_get32();
    NRF_RTC0_NS->EVENTS_COMPARE[0] = 0;
    NRF_RTC0_NS->CC[0] = now + 3;
    NRF_RTC0_NS->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;

    /* Enable PPI */
    NRF_TIMER0_NS->SUBSCRIBE_START = DPPI_SUBSCRIBE_TIMER0_TASKS_START(1);

    /*
     * Store the cputime at which we set the RTC
     *
     * XXX Compare event may be triggered on previous CC value (if it was set to
     * less than N+2) so in rare cases actual start time may be 2 ticks earlier
     * than what we expect. Since this is only used on RX, it may cause AUX scan
     * to be scheduled 1 or 2 ticks too late so we'll miss it - it's acceptable
     * for now.
     */
    g_ble_phy_data.phy_start_cputime = now + 3;

    OS_EXIT_CRITICAL(sr);

    return 0;
}

void
ble_phy_wfr_enable(int txrx, uint8_t tx_phy_mode, uint32_t wfr_usecs)
{
    uint32_t end_time;
    uint8_t phy;

    phy = g_ble_phy_data.phy_cur_phy_mode;

    if (txrx == BLE_PHY_WFR_ENABLE_TXRX) {
        /* RX shall start exactly T_IFS after TX end captured in CC[2] */
        end_time = NRF_TIMER0_NS->CC[2] + BLE_LL_IFS;
        /* Adjust for delay between EVENT_END and actual TX end time */
        end_time += g_ble_phy_t_txenddelay[tx_phy_mode];
        /* Wait a bit longer due to allowed active clock accuracy */
        end_time += 2;
        /*
         * It's possible that we'll capture PDU start time at the end of timer
         * cycle and since wfr expires at the beginning of calculated timer
         * cycle it can be almost 1 usec too early. Let's compensate for this
         * by waiting 1 usec more.
         */
        end_time += 1;
    } else {
        /*
         * RX shall start no later than wfr_usecs after RX enabled.
         * CC[0] is the time of RXEN so adjust for radio ram-up.
         * Do not add jitter since this is already covered by LL.
         */
        end_time = NRF_TIMER0_NS->CC[0] + BLE_PHY_T_RXENFAST + wfr_usecs;
    }

    /*
     * Note: on LE Coded EVENT_ADDRESS is fired after TERM1 is received, so
     *       we are actually calculating relative to start of packet payload
     *       which is fine.
     */

    /* Adjust for receiving access address since this triggers EVENT_ADDRESS */
    end_time += ble_phy_mode_pdu_start_off(phy);
    /* Adjust for delay between actual access address RX and EVENT_ADDRESS */
    end_time += g_ble_phy_t_rxaddrdelay[phy];

    /* wfr_secs is the time from rxen until timeout */
    NRF_TIMER0_NS->CC[3] = end_time;
    NRF_TIMER0_NS->EVENTS_COMPARE[3] = 0;

    /* Subscribe for wait for response events  */
    NRF_TIMER0_NS->SUBSCRIBE_CAPTURE[3] = DPPI_SUBSCRIBE_TIMER0_TASKS_CAPTURE3(1);
    NRF_RADIO_NS->SUBSCRIBE_DISABLE = DPPI_SUBSCRIBE_RADIO_TASKS_DISABLE(1);

    /* Enable the disabled interrupt so we time out on events compare */
    NRF_RADIO_NS->INTENSET = RADIO_INTENSET_DISABLED_Msk;

    /*
     * It may happen that if CPU is halted for a brief moment (e.g. during flash
     * erase or write), TIMER0 already counted past CC[3] and thus wfr will not
     * fire as expected. In case this happened, let's just disable PPIs for wfr
     * and trigger wfr manually (i.e. disable radio).
     *
     * Note that the same applies to RX start time set in CC[0] but since it
     * should fire earlier than wfr, fixing wfr is enough.
     *
     * CC[1] is only used as a reference on RX start, we do not need it here so
     * it can be used to read TIMER0 counter.
     */
    NRF_TIMER0_NS->TASKS_CAPTURE[1] = 1;
    if (NRF_TIMER0_NS->CC[1] > NRF_TIMER0_NS->CC[3]) {
        /* Unsubscribe from wfr events */
        NRF_TIMER0_NS->SUBSCRIBE_CAPTURE[3] = DPPI_SUBSCRIBE_TIMER0_TASKS_CAPTURE3(0);
        NRF_RADIO_NS->SUBSCRIBE_DISABLE = DPPI_SUBSCRIBE_RADIO_TASKS_DISABLE(0);

        NRF_RADIO_NS->TASKS_DISABLE = 1;
    }
}

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
static uint32_t
ble_phy_get_ccm_datarate(void)
{
#if BLE_LL_BT5_PHY_SUPPORTED
    switch (g_ble_phy_data.phy_cur_phy_mode) {
    case BLE_PHY_MODE_1M:
        return CCM_MODE_DATARATE_1Mbit << CCM_MODE_DATARATE_Pos;
    case BLE_PHY_MODE_2M:
        return CCM_MODE_DATARATE_2Mbit << CCM_MODE_DATARATE_Pos;
#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_CODED_PHY)
    case BLE_PHY_MODE_CODED_125KBPS:
        return CCM_MODE_DATARATE_125Kbps << CCM_MODE_DATARATE_Pos;
    case BLE_PHY_MODE_CODED_500KBPS:
        return CCM_MODE_DATARATE_500Kbps << CCM_MODE_DATARATE_Pos;
#endif
    }

    assert(0);
    return 0;
#else
    return CCM_MODE_DATARATE_1Mbit << CCM_MODE_DATARATE_Pos;
#endif
}
#endif

/**
 * Setup transceiver for receive.
 */
static void
ble_phy_rx_xcvr_setup(void)
{
    uint8_t *dptr;

    dptr = (uint8_t *)&g_ble_phy_rx_buf[0];
    dptr += 3;

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
    if (g_ble_phy_data.phy_encrypted) {
        NRF_RADIO_NS->PACKETPTR = (uint32_t)&g_ble_phy_enc_buf[0];
        NRF_CCM_NS->INPTR = (uint32_t)&g_ble_phy_enc_buf[0];
        NRF_CCM_NS->OUTPTR = (uint32_t)dptr;
        NRF_CCM_NS->SCRATCHPTR = (uint32_t)&nrf_encrypt_scratchpad[0];
        NRF_CCM_NS->MODE = CCM_MODE_LENGTH_Msk | CCM_MODE_MODE_Decryption |
                           ble_phy_get_ccm_datarate();
        NRF_CCM_NS->CNFPTR = (uint32_t)&nrf_ccm_data;
        NRF_CCM_NS->SHORTS = 0;
        NRF_CCM_NS->EVENTS_ERROR = 0;
        NRF_CCM_NS->EVENTS_ENDCRYPT = 0;
        NRF_CCM_NS->TASKS_KSGEN = 1;

        /* Subscribe to radio address event */
        NRF_CCM_NS->SUBSCRIBE_CRYPT = DPPI_SUBSCRIBE_CCM_TASKS_CRYPT(1);
    } else {
        NRF_RADIO_NS->PACKETPTR = (uint32_t)dptr;
    }
#else
    NRF_RADIO_NS->PACKETPTR = (uint32_t)dptr;
#endif

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LL_PRIVACY)
    if (g_ble_phy_data.phy_privacy) {
        NRF_AAR_NS->ENABLE = AAR_ENABLE_ENABLE_Enabled;
        NRF_AAR_NS->IRKPTR = (uint32_t)&g_nrf_irk_list[0];
        NRF_AAR_NS->SCRATCHPTR = (uint32_t)&g_ble_phy_data.phy_aar_scratch;
        NRF_AAR_NS->EVENTS_END = 0;
        NRF_AAR_NS->EVENTS_RESOLVED = 0;
        NRF_AAR_NS->EVENTS_NOTRESOLVED = 0;
    } else {
        if (g_ble_phy_data.phy_encrypted == 0) {
            NRF_AAR_NS->ENABLE = AAR_ENABLE_ENABLE_Disabled;
        }
    }
#endif

    /* Turn off trigger TXEN on output compare match and AAR on bcmatch */
    NRF_RADIO_NS->SUBSCRIBE_TXEN = DPPI_SUBSCRIBE_RADIO_TASKS_TXEN(0);
    NRF_AAR_NS->SUBSCRIBE_START = DPPI_SUBSCRIBE_AAR_TASKS_START(0);

    /* Reset the rx started flag. Used for the wait for response */
    g_ble_phy_data.phy_rx_started = 0;
    g_ble_phy_data.phy_state = BLE_PHY_STATE_RX;

#if BLE_LL_BT5_PHY_SUPPORTED
    /*
     * On Coded PHY there are CI and TERM1 fields before PDU starts so we need
     * to take this into account when setting up BCC.
     */
    if (g_ble_phy_data.phy_cur_phy_mode == BLE_PHY_MODE_CODED_125KBPS ||
        g_ble_phy_data.phy_cur_phy_mode == BLE_PHY_MODE_CODED_500KBPS) {
        g_ble_phy_data.phy_bcc_offset = 5;
    } else {
        g_ble_phy_data.phy_bcc_offset = 0;
    }
#else
    g_ble_phy_data.phy_bcc_offset = 0;
#endif

    /* I want to know when 1st byte received (after address) */
    NRF_RADIO_NS->BCC = 8 + g_ble_phy_data.phy_bcc_offset; /* in bits */
    NRF_RADIO_NS->EVENTS_ADDRESS = 0;
    NRF_RADIO_NS->EVENTS_DEVMATCH = 0;
    NRF_RADIO_NS->EVENTS_BCMATCH = 0;
    NRF_RADIO_NS->EVENTS_RSSIEND = 0;
    NRF_RADIO_NS->EVENTS_CRCOK = 0;
    NRF_RADIO_NS->SHORTS = RADIO_SHORTS_END_DISABLE_Msk |
                           RADIO_SHORTS_READY_START_Msk |
                           RADIO_SHORTS_ADDRESS_BCSTART_Msk |
                           RADIO_SHORTS_ADDRESS_RSSISTART_Msk |
                           RADIO_SHORTS_DISABLED_RSSISTOP_Msk;

    NRF_RADIO_NS->INTENSET = RADIO_INTENSET_ADDRESS_Msk;
}

/**
 * Called from interrupt context when the transmit ends
 *
 */
static void
ble_phy_tx_end_isr(void)
{
    uint8_t tx_phy_mode;
    uint8_t was_encrypted;
    uint8_t transition;
    uint32_t rx_time;
    uint32_t wfr_time;

    /* Store PHY on which we've just transmitted smth */
    tx_phy_mode = g_ble_phy_data.phy_cur_phy_mode;

    /* If this transmission was encrypted we need to remember it */
    was_encrypted = g_ble_phy_data.phy_encrypted;
    (void)was_encrypted;

    /* Better be in TX state! */
    assert(g_ble_phy_data.phy_state == BLE_PHY_STATE_TX);

    /* Clear events and clear interrupt on disabled event */
    NRF_RADIO_NS->EVENTS_DISABLED = 0;
    NRF_RADIO_NS->INTENCLR = RADIO_INTENCLR_DISABLED_Msk;
    NRF_RADIO_NS->EVENTS_END = 0;
    wfr_time = NRF_RADIO_NS->SHORTS;
    (void)wfr_time;

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
    /*
     * XXX: not sure what to do. We had a HW error during transmission.
     * For now I just count a stat but continue on like all is good.
     */
    if (was_encrypted) {
        if (NRF_CCM_NS->EVENTS_ERROR) {
            STATS_INC(ble_phy_stats, tx_hw_err);
            NRF_CCM_NS->EVENTS_ERROR = 0;
        }
    }
#endif

    /* Call transmit end callback */
    if (g_ble_phy_data.txend_cb) {
        g_ble_phy_data.txend_cb(g_ble_phy_data.txend_arg);
    }

    transition = g_ble_phy_data.phy_transition;
    if (transition == BLE_PHY_TRANSITION_TX_RX) {

#if (BLE_LL_BT5_PHY_SUPPORTED == 1)
        ble_phy_mode_apply(g_ble_phy_data.phy_rx_phy_mode);
#endif

        /* Packet pointer needs to be reset. */
        ble_phy_rx_xcvr_setup();

        ble_phy_wfr_enable(BLE_PHY_WFR_ENABLE_TXRX, tx_phy_mode, 0);

        /* Schedule RX exactly T_IFS after TX end captured in CC[2] */
        rx_time = NRF_TIMER0_NS->CC[2] + BLE_LL_IFS;
        /* Adjust for delay between EVENT_END and actual TX end time */
        rx_time += g_ble_phy_t_txenddelay[tx_phy_mode];
        /* Adjust for radio ramp-up */
        rx_time -= BLE_PHY_T_RXENFAST;
        /* Start listening a bit earlier due to allowed active clock accuracy */
        rx_time -= 2;

        NRF_TIMER0_NS->CC[0] = rx_time;
        NRF_TIMER0_NS->EVENTS_COMPARE[0] = 0;

        /* Start radio on timer */
        NRF_RADIO_NS->SUBSCRIBE_RXEN = DPPI_SUBSCRIBE_RADIO_TASKS_RXEN(1);

    } else {
        NRF_TIMER0_NS->TASKS_STOP = 1;
        NRF_TIMER0_NS->TASKS_SHUTDOWN = 1;

        NRF_TIMER0_NS->SUBSCRIBE_CAPTURE[3] = DPPI_SUBSCRIBE_TIMER0_TASKS_CAPTURE3(0);
        NRF_RADIO_NS->SUBSCRIBE_DISABLE = DPPI_SUBSCRIBE_RADIO_TASKS_DISABLE(0);
        NRF_RADIO_NS->SUBSCRIBE_TXEN = DPPI_SUBSCRIBE_RADIO_TASKS_TXEN(0);
        NRF_TIMER0_NS->SUBSCRIBE_START = DPPI_SUBSCRIBE_TIMER0_TASKS_START(0);

        assert(transition == BLE_PHY_TRANSITION_NONE);
    }
}

static inline uint8_t
ble_phy_get_cur_rx_phy_mode(void)
{
    uint8_t phy;

    phy = g_ble_phy_data.phy_cur_phy_mode;

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_CODED_PHY)
    /*
     * For Coded PHY mode can be set to either codings since actual coding is
     * set in packet header. However, here we need actual coding of received
     * packet as this determines pipeline delays so need to figure this out
     * using CI field.
     */
    if ((phy == BLE_PHY_MODE_CODED_125KBPS) ||
        (phy == BLE_PHY_MODE_CODED_500KBPS)) {
        phy = NRF_RADIO_NS->PDUSTAT & RADIO_PDUSTAT_CISTAT_Msk ?
              BLE_PHY_MODE_CODED_500KBPS : BLE_PHY_MODE_CODED_125KBPS;
    }
#endif

    return phy;
}

static void
ble_phy_rx_end_isr(void)
{
    int rc;
    uint8_t *dptr;
    uint8_t crcok;
    uint32_t tx_time;
    struct ble_mbuf_hdr *ble_hdr;

    /* Clear events and clear interrupt */
    NRF_RADIO_NS->EVENTS_END = 0;
    NRF_RADIO_NS->INTENCLR = RADIO_INTENCLR_END_Msk;

    /* Disable automatic RXEN */
    NRF_RADIO_NS->SUBSCRIBE_RXEN = DPPI_SUBSCRIBE_RADIO_TASKS_RXEN(0);

    /* Set RSSI and CRC status flag in header */
    ble_hdr = &g_ble_phy_data.rxhdr;
    assert(NRF_RADIO_NS->EVENTS_RSSIEND != 0);
    ble_hdr->rxinfo.rssi = (-1 * NRF_RADIO_NS->RSSISAMPLE) +
                           g_ble_phy_data.rx_pwr_compensation;

    dptr = (uint8_t *)&g_ble_phy_rx_buf[0];
    dptr += 3;

    /* Count PHY crc errors and valid packets */
    crcok = NRF_RADIO_NS->EVENTS_CRCOK;
    if (!crcok) {
        STATS_INC(ble_phy_stats, rx_crc_err);
    } else {
        STATS_INC(ble_phy_stats, rx_valid);
        ble_hdr->rxinfo.flags |= BLE_MBUF_HDR_F_CRC_OK;
#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
        if (g_ble_phy_data.phy_encrypted) {
            /* Only set MIC failure flag if frame is not zero length */
            if ((dptr[1] != 0) && (NRF_CCM_NS->MICSTATUS == 0)) {
                ble_hdr->rxinfo.flags |= BLE_MBUF_HDR_F_MIC_FAILURE;
            }

            /*
             * XXX: not sure how to deal with this. This should not
             * be a MIC failure but we should not hand it up. I guess
             * this is just some form of rx error and that is how we
             * handle it? For now, just set CRC error flags
             */
            if (NRF_CCM_NS->EVENTS_ERROR) {
                STATS_INC(ble_phy_stats, rx_hw_err);
                ble_hdr->rxinfo.flags &= ~BLE_MBUF_HDR_F_CRC_OK;
            }

            /*
             * XXX: This is a total hack work-around for now but I dont
             * know what else to do. If ENDCRYPT is not set and we are
             * encrypted we need to not trust this frame and drop it.
             */
            if (NRF_CCM_NS->EVENTS_ENDCRYPT == 0) {
                STATS_INC(ble_phy_stats, rx_hw_err);
                ble_hdr->rxinfo.flags &= ~BLE_MBUF_HDR_F_CRC_OK;
            }
        }
#endif
    }

#if (BLE_LL_BT5_PHY_SUPPORTED == 1)
    ble_phy_mode_apply(g_ble_phy_data.phy_tx_phy_mode);
#endif

    /*
     * Let's schedule TX now and we will just cancel it after processing RXed
     * packet if we don't need TX.
     *
     * We need this to initiate connection in case AUX_CONNECT_REQ was sent on
     * LE Coded S8. In this case the time we process RXed packet is roughly the
     * same as the limit when we need to have TX scheduled (i.e. TIMER0 and PPI
     * armed) so we may simply miss the slot and set the timer in the past.
     *
     * When TX is scheduled in advance, we may event process packet a bit longer
     * during radio ramp-up - this gives us extra 40 usecs which is more than
     * enough.
     */

    /* Schedule TX exactly T_IFS after RX end captured in CC[2] */
    tx_time = NRF_TIMER0_NS->CC[2] + BLE_LL_IFS;
    /* Adjust for delay between actual RX end time and EVENT_END */
    tx_time -= g_ble_phy_t_rxenddelay[ble_hdr->rxinfo.phy_mode];
    /* Adjust for radio ramp-up */
    tx_time -= BLE_PHY_T_TXENFAST;
    /* Adjust for delay between EVENT_READY and actual TX start time */
    tx_time -= g_ble_phy_t_txdelay[g_ble_phy_data.phy_cur_phy_mode];

    NRF_TIMER0_NS->CC[0] = tx_time;
    NRF_TIMER0_NS->EVENTS_COMPARE[0] = 0;

    /* Enable automatic TX */
    NRF_RADIO_NS->SUBSCRIBE_TXEN = DPPI_SUBSCRIBE_RADIO_TASKS_TXEN(1);

    /*
     * XXX: Hack warning!
     *
     * It may happen (during flash erase) that CPU is stopped for a moment and
     * TIMER0 already counted past CC[0]. In such case we will be stuck waiting
     * for TX to start since EVENTS_COMPARE[0] will not happen any time soon.
     * For now let's set a flag denoting that we are late in RX-TX transition so
     * ble_phy_tx() will fail - this allows everything to cleanup nicely without
     * the need for extra handling in many places.
     *
     * Note: CC[3] is used only for wfr which we do not need here.
     */
    NRF_TIMER0_NS->TASKS_CAPTURE[3] = 1;
    if (NRF_TIMER0_NS->CC[3] > NRF_TIMER0_NS->CC[0]) {
        NRF_RADIO_NS->SUBSCRIBE_TXEN = DPPI_SUBSCRIBE_RADIO_TASKS_TXEN(0);

        g_ble_phy_data.phy_transition_late = 1;
    }

    /*
     * XXX: This is a horrible ugly hack to deal with the RAM S1 byte
     * that is not sent over the air but is present here. Simply move the
     * data pointer to deal with it. Fix this later.
     */
    dptr[2] = dptr[1];
    dptr[1] = dptr[0];
    rc = ble_ll_rx_end(dptr + 1, ble_hdr);
    if (rc < 0) {
        ble_phy_disable();
    }
}

static bool
ble_phy_rx_start_isr(void)
{
    int rc;
    uint32_t state;
    uint32_t usecs;
    uint32_t pdu_usecs;
    uint32_t ticks;
    struct ble_mbuf_hdr *ble_hdr;
    uint8_t *dptr;
#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LL_PRIVACY)
    int adva_offset;
#endif

    dptr = (uint8_t *)&g_ble_phy_rx_buf[0];

    /* Clear events and clear interrupt */
    NRF_RADIO_NS->EVENTS_ADDRESS = 0;

    /* Clear wfr timer channels and DISABLED interrupt */
    NRF_RADIO_NS->INTENCLR = RADIO_INTENCLR_DISABLED_Msk | RADIO_INTENCLR_ADDRESS_Msk;
    NRF_TIMER0_NS->SUBSCRIBE_CAPTURE[3] = DPPI_SUBSCRIBE_TIMER0_TASKS_CAPTURE3(0);
    NRF_RADIO_NS->SUBSCRIBE_DISABLE = DPPI_SUBSCRIBE_RADIO_TASKS_DISABLE(0);

    /* Initialize the ble mbuf header */
    ble_hdr = &g_ble_phy_data.rxhdr;
    ble_hdr->rxinfo.flags = ble_ll_state_get();
    ble_hdr->rxinfo.channel = g_ble_phy_data.phy_chan;
    ble_hdr->rxinfo.handle = 0;
    ble_hdr->rxinfo.phy = ble_phy_get_cur_phy();
    ble_hdr->rxinfo.phy_mode = ble_phy_get_cur_rx_phy_mode();
#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LL_EXT_ADV)
    ble_hdr->rxinfo.user_data = NULL;
#endif

    /*
     * Calculate accurate packets start time (with remainder)
     *
     * We may start receiving packet somewhere during preamble in which case
     * it is possible that actual transmission started before TIMER0 was
     * running - need to take this into account.
     */
    ble_hdr->beg_cputime = g_ble_phy_data.phy_start_cputime;

    usecs = NRF_TIMER0_NS->CC[1];
    pdu_usecs = ble_phy_mode_pdu_start_off(ble_hdr->rxinfo.phy_mode) +
                g_ble_phy_t_rxaddrdelay[ble_hdr->rxinfo.phy_mode];
    if (usecs < pdu_usecs) {
        g_ble_phy_data.phy_start_cputime--;
        usecs += 30;
    }
    usecs -= pdu_usecs;

    ticks = os_cputime_usecs_to_ticks(usecs);
    usecs -= os_cputime_ticks_to_usecs(ticks);
    if (usecs == 31) {
        usecs = 0;
        ++ticks;
    }

    ble_hdr->beg_cputime += ticks;
    ble_hdr->rem_usecs = usecs;

    /* Wait to get 1st byte of frame */
    while (1) {
        state = NRF_RADIO_NS->STATE;
        if (NRF_RADIO_NS->EVENTS_BCMATCH != 0) {
            break;
        }

        /*
         * If state is disabled, we should have the BCMATCH. If not,
         * something is wrong!
         */
        if (state == RADIO_STATE_STATE_Disabled) {
            NRF_RADIO_NS->INTENCLR = NRF_RADIO_IRQ_MASK_ALL;
            NRF_RADIO_NS->SHORTS = 0;
            return false;
        }
    }

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LL_PRIVACY)
    /*
     * If privacy is enabled and received PDU has TxAdd bit set (i.e. random
     * address) we try to resolve address using AAR.
     */
    if (g_ble_phy_data.phy_privacy && (dptr[3] & 0x40)) {
        /*
         * AdvA is located at 4th octet in RX buffer (after S0, length an S1
         * fields). In case of extended advertising PDU we need to add 2 more
         * octets for extended header.
         */
        adva_offset = (dptr[3] & 0x0f) == 0x07 ? 2 : 0;
        NRF_AAR_NS->ADDRPTR = (uint32_t)(dptr + 3 + adva_offset);

        /* Trigger AAR after last bit of AdvA is received */
        NRF_RADIO_NS->EVENTS_BCMATCH = 0;
        NRF_AAR_NS->SUBSCRIBE_START = DPPI_SUBSCRIBE_AAR_TASKS_START(1);
        NRF_RADIO_NS->BCC = (BLE_LL_PDU_HDR_LEN + adva_offset + BLE_DEV_ADDR_LEN) * 8 +
                            g_ble_phy_data.phy_bcc_offset;
    }
#endif

    /* Call Link Layer receive start function */
    rc = ble_ll_rx_start(dptr + 3,
                         g_ble_phy_data.phy_chan,
                         &g_ble_phy_data.rxhdr);
    if (rc >= 0) {
        /* Set rx started flag and enable rx end ISR */
        g_ble_phy_data.phy_rx_started = 1;
        NRF_RADIO_NS->INTENSET = RADIO_INTENSET_END_Msk;
    } else {
        /* Disable PHY */
        ble_phy_disable();
        STATS_INC(ble_phy_stats, rx_aborts);
    }

    /* Count rx starts */
    STATS_INC(ble_phy_stats, rx_starts);

    return true;
}

static void
ble_phy_isr(void)
{
    uint32_t irq_en;

    os_trace_isr_enter();

    /* Read irq register to determine which interrupts are enabled */
    irq_en = NRF_RADIO_NS->INTENCLR;

    /*
     * NOTE: order of checking is important! Possible, if things get delayed,
     * we have both an ADDRESS and DISABLED interrupt in rx state. If we get
     * an address, we disable the DISABLED interrupt.
     */

    /* We get this if we have started to receive a frame */
    if ((irq_en & RADIO_INTENCLR_ADDRESS_Msk) && NRF_RADIO_NS->EVENTS_ADDRESS) {
        /*
         * wfr timer is calculated to expire at the exact time we should start
         * receiving a packet (with 1 usec precision) so it is possible  it will
         * fire at the same time as EVENT_ADDRESS. If this happens, radio will
         * be disabled while we are waiting for EVENT_BCCMATCH after 1st byte
         * of payload is received and ble_phy_rx_start_isr() will fail. In this
         * case we should not clear DISABLED irq mask so it will be handled as
         * regular radio disabled event below. In other case radio was disabled
         * on purpose and there's nothing more to handle so we can clear mask.
         */
        if (ble_phy_rx_start_isr()) {
            irq_en &= ~RADIO_INTENCLR_DISABLED_Msk;
        }
    }

    /* Check for disabled event. This only happens for transmits now */
    if ((irq_en & RADIO_INTENCLR_DISABLED_Msk) && NRF_RADIO_NS->EVENTS_DISABLED) {
        if (g_ble_phy_data.phy_state == BLE_PHY_STATE_RX) {
            NRF_RADIO_NS->EVENTS_DISABLED = 0;
            ble_ll_wfr_timer_exp(NULL);
        } else if (g_ble_phy_data.phy_state == BLE_PHY_STATE_IDLE) {
            assert(0);
        } else {
            ble_phy_tx_end_isr();
        }
    }

    /* Receive packet end (we dont enable this for transmit) */
    if ((irq_en & RADIO_INTENCLR_END_Msk) && NRF_RADIO_NS->EVENTS_END) {
        ble_phy_rx_end_isr();
    }

    g_ble_phy_data.phy_transition_late = 0;

    /* Ensures IRQ is cleared */
    irq_en = NRF_RADIO_NS->SHORTS;

    /* Count # of interrupts */
    STATS_INC(ble_phy_stats, phy_isrs);

    os_trace_isr_exit();
}

int
ble_phy_init(void)
{
    int rc;

    /* Default phy to use is 1M */
    g_ble_phy_data.phy_cur_phy_mode = BLE_PHY_MODE_1M;
    g_ble_phy_data.phy_tx_phy_mode = BLE_PHY_MODE_1M;
    g_ble_phy_data.phy_rx_phy_mode = BLE_PHY_MODE_1M;

    g_ble_phy_data.rx_pwr_compensation = 0;

    /* Set phy channel to an invalid channel so first set channel works */
    g_ble_phy_data.phy_chan = BLE_PHY_NUM_CHANS;

    /* Toggle peripheral power to reset (just in case) */
    NRF_RADIO_NS->POWER = 0;
    NRF_RADIO_NS->POWER = 1;

    /* Errata 16 - RADIO: POWER register is not functional
     * Workaround: Reset all RADIO registers in firmware.
     */
    NRF_RADIO_NS->SUBSCRIBE_TXEN = 0;
    NRF_RADIO_NS->SUBSCRIBE_RXEN = 0;
    NRF_RADIO_NS->SUBSCRIBE_DISABLE = 0;

    /* Disable all interrupts */
    NRF_RADIO_NS->INTENCLR = NRF_RADIO_IRQ_MASK_ALL;

    /* Set configuration registers */
    NRF_RADIO_NS->MODE = RADIO_MODE_MODE_Ble_1Mbit;
    NRF_RADIO_NS->PCNF0 = NRF_PCNF0;

    /* XXX: should maxlen be 251 for encryption? */
    NRF_RADIO_NS->PCNF1 = NRF_MAXLEN |
                          (RADIO_PCNF1_ENDIAN_Little <<  RADIO_PCNF1_ENDIAN_Pos) |
                          (NRF_BALEN << RADIO_PCNF1_BALEN_Pos) |
                          RADIO_PCNF1_WHITEEN_Msk;

    /* Enable radio fast ramp-up */
    NRF_RADIO_NS->MODECNF0 |= (RADIO_MODECNF0_RU_Fast << RADIO_MODECNF0_RU_Pos) & RADIO_MODECNF0_RU_Msk;

    /* Set logical address 1 for TX and RX */
    NRF_RADIO_NS->TXADDRESS = 0;
    NRF_RADIO_NS->RXADDRESSES = (1 << 0);

    /* Configure the CRC registers */
    NRF_RADIO_NS->CRCCNF = (RADIO_CRCCNF_SKIPADDR_Skip << RADIO_CRCCNF_SKIPADDR_Pos) | RADIO_CRCCNF_LEN_Three;

    /* Configure BLE poly */
    NRF_RADIO_NS->CRCPOLY = 0x0000065B;

    /* Configure IFS */
    NRF_RADIO_NS->TIFS = BLE_LL_IFS;

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
    NRF_CCM_NS->INTENCLR = 0xffffffff;
    NRF_CCM_NS->SHORTS = CCM_SHORTS_ENDKSGEN_CRYPT_Msk;
    NRF_CCM_NS->EVENTS_ERROR = 0;
    memset(nrf_encrypt_scratchpad, 0, sizeof(nrf_encrypt_scratchpad));
#endif

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LL_PRIVACY)
    g_ble_phy_data.phy_aar_scratch = 0;
    NRF_AAR_NS->IRKPTR = (uint32_t)&g_nrf_irk_list[0];
    NRF_AAR_NS->INTENCLR = 0xffffffff;
    NRF_AAR_NS->EVENTS_END = 0;
    NRF_AAR_NS->EVENTS_RESOLVED = 0;
    NRF_AAR_NS->EVENTS_NOTRESOLVED = 0;
    NRF_AAR_NS->NIRK = 0;
#endif

    /* TIMER0 setup for PHY when using RTC */
    NRF_TIMER0_NS->TASKS_STOP = 1;
    NRF_TIMER0_NS->TASKS_SHUTDOWN = 1;
    NRF_TIMER0_NS->BITMODE = 3;    /* 32-bit timer */
    NRF_TIMER0_NS->MODE = 0;       /* Timer mode */
    NRF_TIMER0_NS->PRESCALER = 4;  /* gives us 1 MHz */

    /* Publish events */
    NRF_TIMER0_NS->PUBLISH_COMPARE[0] = DPPI_PUBLISH_TIMER0_EVENTS_COMPARE_0;
    NRF_TIMER0_NS->PUBLISH_COMPARE[3] = DPPI_PUBLISH_TIMER0_EVENTS_COMPARE_3;
    NRF_RADIO_NS->PUBLISH_END = DPPI_PUBLISH_RADIO_EVENTS_END;
    NRF_RADIO_NS->PUBLISH_BCMATCH = DPPI_PUBLISH_RADIO_EVENTS_BCMATCH;
    NRF_RADIO_NS->PUBLISH_ADDRESS = DPPI_PUBLISH_RADIO_EVENTS_ADDRESS;
    NRF_RTC0_NS->PUBLISH_COMPARE[0] = DPPI_PUBLISH_RTC0_EVENTS_COMPARE_0;

    /* Enable channels we publish on */
    NRF_DPPIC_NS->CHENSET = DPPI_CH_ENABLE_ALL;

    /* Captures tx/rx start in timer0 cc 1 and tx/rx end in timer0 cc 2 */
    NRF_TIMER0_NS->SUBSCRIBE_CAPTURE[1] = DPPI_SUBSCRIBE_TIMER0_TASKS_CAPTURE1(1);
    NRF_TIMER0_NS->SUBSCRIBE_CAPTURE[2] = DPPI_SUBSCRIBE_TIMER0_TASKS_CAPTURE2(1);

    /* Set isr in vector table and enable interrupt */
#ifndef RIOT_VERSION
    NVIC_SetPriority(RADIO_IRQn, 0);
#endif
#if MYNEWT
    NVIC_SetVector(RADIO_IRQn, (uint32_t)ble_phy_isr);
#else
    ble_npl_hw_set_isr(RADIO_IRQn, ble_phy_isr);
#endif
    NVIC_EnableIRQ(RADIO_IRQn);

    /* Register phy statistics */
    if (!g_ble_phy_data.phy_stats_initialized) {
        rc = stats_init_and_reg(STATS_HDR(ble_phy_stats),
                                STATS_SIZE_INIT_PARMS(ble_phy_stats,
                                                      STATS_SIZE_32),
                                STATS_NAME_INIT_PARMS(ble_phy_stats),
                                "ble_phy");
        assert(rc == 0);

        g_ble_phy_data.phy_stats_initialized = 1;
    }

    return 0;
}

int
ble_phy_rx(void)
{
    /*
     * Check radio state.
     *
     * In case radio is now disabling we'll wait for it to finish, but if for
     * any reason it's just in idle state we proceed with RX as usual since
     * nRF52 radio can ramp-up from idle state as well.
     *
     * Note that TX and RX states values are the same except for 3rd bit so we
     * can make a shortcut here when checking for idle state.
     */
    nrf_wait_disabled();
    if ((NRF_RADIO_NS->STATE != RADIO_STATE_STATE_Disabled) &&
        ((NRF_RADIO_NS->STATE & 0x07) != RADIO_STATE_STATE_RxIdle)) {
        ble_phy_disable();
        STATS_INC(ble_phy_stats, radio_state_errs);
        return BLE_PHY_ERR_RADIO_STATE;
    }

    /* Make sure all interrupts are disabled */
    NRF_RADIO_NS->INTENCLR = NRF_RADIO_IRQ_MASK_ALL;

    /* Clear events prior to enabling receive */
    NRF_RADIO_NS->EVENTS_END = 0;
    NRF_RADIO_NS->EVENTS_DISABLED = 0;

    /* Setup for rx */
    ble_phy_rx_xcvr_setup();

    /* task to start RX should be subscribed here  */
    assert(NRF_RADIO_NS->SUBSCRIBE_RXEN & DPPI_SUBSCRIBE_RADIO_TASKS_RXEN(1));

    return 0;
}

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
void
ble_phy_encrypt_enable(uint64_t pkt_counter, uint8_t *iv, uint8_t *key,
                       uint8_t is_master)
{
    memcpy(nrf_ccm_data.key, key, 16);
    nrf_ccm_data.pkt_counter = pkt_counter;
    memcpy(nrf_ccm_data.iv, iv, 8);
    nrf_ccm_data.dir_bit = is_master;
    g_ble_phy_data.phy_encrypted = 1;
    /* Enable the module (AAR cannot be on while CCM on) */
    NRF_AAR_NS->ENABLE = AAR_ENABLE_ENABLE_Disabled;
    NRF_CCM_NS->ENABLE = CCM_ENABLE_ENABLE_Enabled;
}

void
ble_phy_encrypt_set_pkt_cntr(uint64_t pkt_counter, int dir)
{
    nrf_ccm_data.pkt_counter = pkt_counter;
    nrf_ccm_data.dir_bit = dir;
}

void
ble_phy_encrypt_disable(void)
{
    NRF_CCM_NS->SUBSCRIBE_CRYPT = DPPI_SUBSCRIBE_CCM_TASKS_CRYPT(0);
    NRF_CCM_NS->TASKS_STOP = 1;
    NRF_CCM_NS->EVENTS_ERROR = 0;
    NRF_CCM_NS->ENABLE = CCM_ENABLE_ENABLE_Disabled;

    g_ble_phy_data.phy_encrypted = 0;
}
#endif

void
ble_phy_set_txend_cb(ble_phy_tx_end_func txend_cb, void *arg)
{
    /* Set transmit end callback and arg */
    g_ble_phy_data.txend_cb = txend_cb;
    g_ble_phy_data.txend_arg = arg;
}

int
ble_phy_tx_set_start_time(uint32_t cputime, uint8_t rem_usecs)
{
    int rc;

    ble_phy_trace_u32x2(BLE_PHY_TRACE_ID_START_TX, cputime, rem_usecs);

#if (BLE_LL_BT5_PHY_SUPPORTED == 1)
    ble_phy_mode_apply(g_ble_phy_data.phy_tx_phy_mode);
#endif

    /* XXX: This should not be necessary, but paranoia is good! */
    /* Clear timer0 compare to RXEN since we are transmitting */
    NRF_RADIO_NS->SUBSCRIBE_RXEN = DPPI_SUBSCRIBE_RADIO_TASKS_RXEN(0);

    if (ble_phy_set_start_time(cputime, rem_usecs, true) != 0) {
        STATS_INC(ble_phy_stats, tx_late);
        ble_phy_disable();
        rc = BLE_PHY_ERR_TX_LATE;
    } else {
        /* Enable PPI to automatically start TXEN */
        NRF_RADIO_NS->SUBSCRIBE_TXEN = DPPI_SUBSCRIBE_RADIO_TASKS_TXEN(1);
        rc = 0;
    }
    return rc;
}

int
ble_phy_rx_set_start_time(uint32_t cputime, uint8_t rem_usecs)
{
    bool late = false;
    int rc = 0;

    ble_phy_trace_u32x2(BLE_PHY_TRACE_ID_START_RX, cputime, rem_usecs);

#if (BLE_LL_BT5_PHY_SUPPORTED == 1)
    ble_phy_mode_apply(g_ble_phy_data.phy_rx_phy_mode);
#endif

    /* XXX: This should not be necessary, but paranoia is good! */
    /* Clear timer0 compare to TXEN since we are transmitting */
    NRF_RADIO_NS->SUBSCRIBE_TXEN = DPPI_SUBSCRIBE_RADIO_TASKS_TXEN(0);

    if (ble_phy_set_start_time(cputime, rem_usecs, false) != 0) {
        STATS_INC(ble_phy_stats, rx_late);

        /* We're late so let's just try to start RX as soon as possible */
        ble_phy_set_start_now();

        late = true;
    }

    /* Enable PPI to automatically start RXEN */
    NRF_RADIO_NS->SUBSCRIBE_RXEN = DPPI_SUBSCRIBE_RADIO_TASKS_RXEN(1);

    /* Start rx */
    rc = ble_phy_rx();

    /*
     * If we enabled receiver but were late, let's return proper error code so
     * caller can handle this.
     */
    if (!rc && late) {
        rc = BLE_PHY_ERR_RX_LATE;
    }

    return rc;
}

int
ble_phy_tx(ble_phy_tx_pducb_t pducb, void *pducb_arg, uint8_t end_trans)
{
    int rc;
    uint8_t *dptr;
    uint8_t *pktptr;
    uint8_t payload_len;
    uint8_t hdr_byte;
    uint32_t state;
    uint32_t shortcuts;

    if (g_ble_phy_data.phy_transition_late) {
        ble_phy_disable();
        STATS_INC(ble_phy_stats, tx_late);
        return BLE_PHY_ERR_TX_LATE;
    }

    /*
     * This check is to make sure that the radio is not in a state where
     * it is moving to disabled state. If so, let it get there.
     */
    nrf_wait_disabled();

    /*
     * XXX: Although we may not have to do this here, I clear all the PPI
     * that should not be used when transmitting. Some of them are only enabled
     * if encryption and/or privacy is on, but I dont care. Better to be
     * paranoid, and if you are going to clear one, might as well clear them
     * all.
     */
    NRF_TIMER0_NS->SUBSCRIBE_CAPTURE[3] = DPPI_SUBSCRIBE_TIMER0_TASKS_CAPTURE3(0);
    NRF_RADIO_NS->SUBSCRIBE_DISABLE = DPPI_SUBSCRIBE_RADIO_TASKS_DISABLE(0);
    NRF_AAR_NS->SUBSCRIBE_START = DPPI_SUBSCRIBE_AAR_TASKS_START(0);
    NRF_CCM_NS->SUBSCRIBE_CRYPT = DPPI_SUBSCRIBE_CCM_TASKS_CRYPT(0);

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
    if (g_ble_phy_data.phy_encrypted) {
        dptr = (uint8_t *)&g_ble_phy_enc_buf[0];
        pktptr = (uint8_t *)&g_ble_phy_tx_buf[0];
        NRF_CCM_NS->SHORTS = CCM_SHORTS_ENDKSGEN_CRYPT_Msk;
        NRF_CCM_NS->INPTR = (uint32_t)dptr;
        NRF_CCM_NS->OUTPTR = (uint32_t)pktptr;
        NRF_CCM_NS->SCRATCHPTR = (uint32_t)&nrf_encrypt_scratchpad[0];
        NRF_CCM_NS->EVENTS_ERROR = 0;
        NRF_CCM_NS->MODE = CCM_MODE_LENGTH_Msk | ble_phy_get_ccm_datarate();
        NRF_CCM_NS->CNFPTR = (uint32_t)&nrf_ccm_data;
    } else {
#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LL_PRIVACY)
        NRF_AAR_NS->IRKPTR = (uint32_t)&g_nrf_irk_list[0];
#endif
        dptr = (uint8_t *)&g_ble_phy_tx_buf[0];
        pktptr = dptr;
    }
#else
    dptr = (uint8_t *)&g_ble_phy_tx_buf[0];
    pktptr = dptr;
#endif

    /* Set PDU payload */
    payload_len = pducb(&dptr[3], pducb_arg, &hdr_byte);

    /* RAM representation has S0, LENGTH and S1 fields. (3 bytes) */
    dptr[0] = hdr_byte;
    dptr[1] = payload_len;
    dptr[2] = 0;

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LE_ENCRYPTION)
    /* Start key-stream generation and encryption (via short) */
    if (g_ble_phy_data.phy_encrypted) {
        NRF_CCM_NS->TASKS_KSGEN = 1;
    }
#endif

    NRF_RADIO_NS->PACKETPTR = (uint32_t)pktptr;

    /* Clear the ready, end and disabled events */
    NRF_RADIO_NS->EVENTS_READY = 0;
    NRF_RADIO_NS->EVENTS_END = 0;
    NRF_RADIO_NS->EVENTS_DISABLED = 0;

    /* Enable shortcuts for transmit start/end. */
    shortcuts = RADIO_SHORTS_END_DISABLE_Msk | RADIO_SHORTS_READY_START_Msk;
    NRF_RADIO_NS->SHORTS = shortcuts;
    NRF_RADIO_NS->INTENSET = RADIO_INTENSET_DISABLED_Msk;

    /* Set the PHY transition */
    g_ble_phy_data.phy_transition = end_trans;

    /* Set transmitted payload length */
    g_ble_phy_data.phy_tx_pyld_len = payload_len;

    /* If we already started transmitting, abort it! */
    state = NRF_RADIO_NS->STATE;
    if (state != RADIO_STATE_STATE_Tx) {
        /* Set phy state to transmitting and count packet statistics */
        g_ble_phy_data.phy_state = BLE_PHY_STATE_TX;
        STATS_INC(ble_phy_stats, tx_good);
        STATS_INCN(ble_phy_stats, tx_bytes, payload_len + BLE_LL_PDU_HDR_LEN);
        rc = BLE_ERR_SUCCESS;
    } else {
        ble_phy_disable();
        STATS_INC(ble_phy_stats, tx_late);
        rc = BLE_PHY_ERR_RADIO_STATE;
    }

    return rc;
}

int
ble_phy_txpwr_set(int dbm)
{
    /* "Rail" power level if outside supported range */
    dbm = ble_phy_txpower_round(dbm);

    NRF_RADIO_NS->TXPOWER = dbm;
    g_ble_phy_data.phy_txpwr_dbm = dbm;

    return 0;
}

int
ble_phy_txpower_round(int dbm)
{
    /* "Rail" power level if outside supported range */

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_0dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_0dBm;
    }

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_Neg1dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_Neg1dBm;
    }

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_Neg2dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_Neg2dBm;
    }

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_Neg3dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_Neg4dBm;
    }

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_Neg4dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_Neg4dBm;
    }

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_Neg5dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_Neg5dBm;
    }

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_Neg6dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_Neg6dBm;
    }

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_Neg7dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_Neg7dBm;
    }

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_Neg8dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_Neg8dBm;
    }

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_Neg12dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_Neg12dBm;
    }

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_Neg16dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_Neg16dBm;
    }

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_Neg20dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_Neg20dBm;
    }

    if (dbm >= (int8_t)RADIO_TXPOWER_TXPOWER_Neg40dBm) {
        return (int8_t)RADIO_TXPOWER_TXPOWER_Neg40dBm;
    }

    return (int8_t)RADIO_TXPOWER_TXPOWER_Neg40dBm;
}

static int
ble_phy_set_access_addr(uint32_t access_addr)
{
    NRF_RADIO_NS->BASE0 = (access_addr << 8);
    NRF_RADIO_NS->PREFIX0 = (NRF_RADIO_NS->PREFIX0 & 0xFFFFFF00) | (access_addr >> 24);

    g_ble_phy_data.phy_access_address = access_addr;

    return 0;
}

int
ble_phy_txpwr_get(void)
{
    return g_ble_phy_data.phy_txpwr_dbm;
}

void
ble_phy_set_rx_pwr_compensation(int8_t compensation)
{
    g_ble_phy_data.rx_pwr_compensation = compensation;
}

int
ble_phy_setchan(uint8_t chan, uint32_t access_addr, uint32_t crcinit)
{
    assert(chan < BLE_PHY_NUM_CHANS);

    /* Check for valid channel range */
    if (chan >= BLE_PHY_NUM_CHANS) {
        return BLE_PHY_ERR_INV_PARAM;
    }

    /* Set current access address */
    ble_phy_set_access_addr(access_addr);

    /* Configure crcinit */
    NRF_RADIO_NS->CRCINIT = crcinit;

    /* Set the frequency and the data whitening initial value */
    g_ble_phy_data.phy_chan = chan;
    NRF_RADIO_NS->FREQUENCY = g_ble_phy_chan_freq[chan];
    NRF_RADIO_NS->DATAWHITEIV = chan;

    return 0;
}

/**
 * Stop the timer used to count microseconds when using RTC for cputime
 */
static void
ble_phy_stop_usec_timer(void)
{
    NRF_TIMER0_NS->TASKS_STOP = 1;
    NRF_TIMER0_NS->TASKS_SHUTDOWN = 1;
    NRF_RTC0_NS->EVTENCLR = RTC_EVTENSET_COMPARE0_Msk;
}

/**
 * ble phy disable irq and ppi
 *
 * This routine is to be called when reception was stopped due to either a
 * wait for response timeout or a packet being received and the phy is to be
 * restarted in receive mode. Generally, the disable routine is called to stop
 * the phy.
 */
static void
ble_phy_disable_irq_and_ppi(void)
{
    NRF_RADIO_NS->INTENCLR = NRF_RADIO_IRQ_MASK_ALL;
    NRF_RADIO_NS->SHORTS = 0;
    NRF_RADIO_NS->TASKS_DISABLE = 1;

    NRF_TIMER0_NS->SUBSCRIBE_START = DPPI_SUBSCRIBE_TIMER0_TASKS_START(0);
    NRF_TIMER0_NS->SUBSCRIBE_CAPTURE[3] = DPPI_SUBSCRIBE_TIMER0_TASKS_CAPTURE3(0);
    NRF_RADIO_NS->SUBSCRIBE_DISABLE = DPPI_SUBSCRIBE_RADIO_TASKS_DISABLE(0);
    NRF_RADIO_NS->SUBSCRIBE_TXEN = DPPI_SUBSCRIBE_RADIO_TASKS_TXEN(0);
    NRF_RADIO_NS->SUBSCRIBE_RXEN = DPPI_SUBSCRIBE_RADIO_TASKS_RXEN(0);
    NRF_AAR_NS->SUBSCRIBE_START = DPPI_SUBSCRIBE_AAR_TASKS_START(0);
    NRF_CCM_NS->SUBSCRIBE_CRYPT = DPPI_SUBSCRIBE_CCM_TASKS_CRYPT(0);

    NVIC_ClearPendingIRQ(RADIO_IRQn);
    g_ble_phy_data.phy_state = BLE_PHY_STATE_IDLE;
}

void
ble_phy_restart_rx(void)
{
    ble_phy_stop_usec_timer();
    ble_phy_disable_irq_and_ppi();

    ble_phy_set_start_now();
    /* Enable PPI to automatically start RXEN */
    NRF_RADIO_NS->SUBSCRIBE_RXEN = DPPI_SUBSCRIBE_RADIO_TASKS_RXEN(1);

    ble_phy_rx();
}

void
ble_phy_disable(void)
{
    ble_phy_trace_void(BLE_PHY_TRACE_ID_DISABLE);

    ble_phy_stop_usec_timer();
    ble_phy_disable_irq_and_ppi();
}

uint32_t
ble_phy_access_addr_get(void)
{
    return g_ble_phy_data.phy_access_address;
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
    uint32_t state;
    state = NRF_RADIO_NS->STATE;
    return (uint8_t)state;
}

uint8_t
ble_phy_max_data_pdu_pyld(void)
{
    return BLE_LL_DATA_PDU_MAX_PYLD;
}

#if MYNEWT_VAL(BLE_LL_CFG_FEAT_LL_PRIVACY)
void
ble_phy_resolv_list_enable(void)
{
    NRF_AAR_NS->NIRK = (uint32_t)g_nrf_num_irks;
    g_ble_phy_data.phy_privacy = 1;
}

void
ble_phy_resolv_list_disable(void)
{
    g_ble_phy_data.phy_privacy = 0;
}
#endif

#if MYNEWT_VAL(BLE_LL_DTM)
void
ble_phy_enable_dtm(void)
{
    /* When DTM is enabled we need to disable whitening as per
     * Bluetooth v5.0 Vol 6. Part F. 4.1.1
     */
    NRF_RADIO_NS->PCNF1 &= ~RADIO_PCNF1_WHITEEN_Msk;
}

void
ble_phy_disable_dtm(void)
{
    /* Enable whitening */
    NRF_RADIO_NS->PCNF1 |= RADIO_PCNF1_WHITEEN_Msk;
}
#endif

void
ble_phy_rfclk_enable(void)
{
#if MYNEWT
    nrf5340_net_clock_hfxo_request();
#else
    NRF_CLOCK_NS->TASKS_HFCLKSTART = 1;
#endif
}

void
ble_phy_rfclk_disable(void)
{
#if MYNEWT
    nrf5340_net_clock_hfxo_release();
#else
    NRF_CLOCK_NS->TASKS_HFCLKSTOP = 1;
#endif
}
