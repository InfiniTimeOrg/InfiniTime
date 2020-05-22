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

#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include "os/os.h"
#include "nrfx.h"
#include "hal/hal_timer.h"

/* IRQ prototype */
typedef void (*hal_timer_irq_handler_t)(void);

/* User CC 2 for reading counter, CC 3 for timer isr */
#define NRF_TIMER_CC_READ       (2)
#define NRF_TIMER_CC_INT        (3)

/* Output compare 2 used for RTC timers */
#define NRF_RTC_TIMER_CC_INT    (2)

/* Maximum number of hal timers used */
#define NRF52_HAL_TIMER_MAX     (6)

/* Maximum timer frequency */
#define NRF52_MAX_TIMER_FREQ    (16000000)

struct nrf52_hal_timer {
    uint8_t tmr_enabled;
    uint8_t tmr_irq_num;
    uint8_t tmr_rtc;
    uint8_t tmr_pad;
    uint32_t tmr_cntr;
    uint32_t timer_isrs;
    uint32_t tmr_freq;
    void *tmr_reg;
    TAILQ_HEAD(hal_timer_qhead, hal_timer) hal_timer_q;
};

#if MYNEWT_VAL(TIMER_0)
struct nrf52_hal_timer nrf52_hal_timer0;
#endif
#if MYNEWT_VAL(TIMER_1)
struct nrf52_hal_timer nrf52_hal_timer1;
#endif
#if MYNEWT_VAL(TIMER_2)
struct nrf52_hal_timer nrf52_hal_timer2;
#endif
#if MYNEWT_VAL(TIMER_3)
struct nrf52_hal_timer nrf52_hal_timer3;
#endif
#if MYNEWT_VAL(TIMER_4)
struct nrf52_hal_timer nrf52_hal_timer4;
#endif
#if MYNEWT_VAL(TIMER_5)
struct nrf52_hal_timer nrf52_hal_timer5;
#endif

static const struct nrf52_hal_timer *nrf52_hal_timers[NRF52_HAL_TIMER_MAX] = {
#if MYNEWT_VAL(TIMER_0)
    &nrf52_hal_timer0,
#else
    NULL,
#endif
#if MYNEWT_VAL(TIMER_1)
    &nrf52_hal_timer1,
#else
    NULL,
#endif
#if MYNEWT_VAL(TIMER_2)
    &nrf52_hal_timer2,
#else
    NULL,
#endif
#if MYNEWT_VAL(TIMER_3)
    &nrf52_hal_timer3,
#else
    NULL,
#endif
#if MYNEWT_VAL(TIMER_4)
    &nrf52_hal_timer4,
#else
    NULL,
#endif
#if MYNEWT_VAL(TIMER_5)
    &nrf52_hal_timer5
#else
    NULL
#endif
};

/* Resolve timer number into timer structure */
#define NRF52_HAL_TIMER_RESOLVE(__n, __v)       \
    if ((__n) >= NRF52_HAL_TIMER_MAX) {         \
        rc = EINVAL;                            \
        goto err;                               \
    }                                           \
    (__v) = (struct nrf52_hal_timer *) nrf52_hal_timers[(__n)];            \
    if ((__v) == NULL) {                        \
        rc = EINVAL;                            \
        goto err;                               \
    }

/* Interrupt mask for interrupt enable/clear */
#define NRF_TIMER_INT_MASK(x)    ((1 << (uint32_t)(x)) << 16)

static uint32_t
nrf_read_timer_cntr(NRF_TIMER_Type *hwtimer)
{
    uint32_t tcntr;

    /* Force a capture of the timer into 'cntr' capture channel; read it */
    hwtimer->TASKS_CAPTURE[NRF_TIMER_CC_READ] = 1;
    tcntr = hwtimer->CC[NRF_TIMER_CC_READ];

    return tcntr;
}

/**
 * nrf timer set ocmp
 *
 * Set the OCMP used by the timer to the desired expiration tick
 *
 * NOTE: Must be called with interrupts disabled.
 *
 * @param timer Pointer to timer.
 */
static void
nrf_timer_set_ocmp(struct nrf52_hal_timer *bsptimer, uint32_t expiry)
{
    int32_t delta_t;
    uint32_t temp;
    uint32_t cntr;
    NRF_TIMER_Type *hwtimer;
    NRF_RTC_Type *rtctimer;

    if (bsptimer->tmr_rtc) {
        rtctimer = (NRF_RTC_Type *)bsptimer->tmr_reg;
        rtctimer->INTENCLR = NRF_TIMER_INT_MASK(NRF_RTC_TIMER_CC_INT);
        temp = bsptimer->tmr_cntr;
        cntr = rtctimer->COUNTER;
        if (rtctimer->EVENTS_OVRFLW) {
            temp += (1UL << 24);
            cntr = rtctimer->COUNTER;
        }
        temp |= cntr;
        delta_t = (int32_t)(expiry - temp);

        /*
         * The nrf documentation states that you must set the output
         * compare to 2 greater than the counter to guarantee an interrupt.
         * Since the counter can tick once while we check, we make sure
         * it is greater than 2.
         */
        if (delta_t < 3) {
            NVIC_SetPendingIRQ(bsptimer->tmr_irq_num);
        } else  {
            if (delta_t < (1UL << 24)) {
                rtctimer->CC[NRF_RTC_TIMER_CC_INT] = expiry & 0x00ffffff;
            } else {
                /* CC too far ahead. Just make sure we set compare far ahead */
                rtctimer->CC[NRF_RTC_TIMER_CC_INT] = cntr + (1UL << 23);
            }
            rtctimer->INTENSET = NRF_TIMER_INT_MASK(NRF_RTC_TIMER_CC_INT);
        }
    } else {
        hwtimer = bsptimer->tmr_reg;

        /* Disable ocmp interrupt and set new value */
        hwtimer->INTENCLR = NRF_TIMER_INT_MASK(NRF_TIMER_CC_INT);

        /* Set output compare register to timer expiration */
        hwtimer->CC[NRF_TIMER_CC_INT] = expiry;

        /* Clear interrupt flag */
        hwtimer->EVENTS_COMPARE[NRF_TIMER_CC_INT] = 0;

        /* Enable the output compare interrupt */
        hwtimer->INTENSET = NRF_TIMER_INT_MASK(NRF_TIMER_CC_INT);

        /* Force interrupt to occur as we may have missed it */
        if ((int32_t)(nrf_read_timer_cntr(hwtimer) - expiry) >= 0) {
            NVIC_SetPendingIRQ(bsptimer->tmr_irq_num);
        }
    }
}

/* Disable output compare used for timer */
static void
nrf_timer_disable_ocmp(NRF_TIMER_Type *hwtimer)
{
    hwtimer->INTENCLR = NRF_TIMER_INT_MASK(NRF_TIMER_CC_INT);
}

static void
nrf_rtc_disable_ocmp(NRF_RTC_Type *rtctimer)
{
    rtctimer->INTENCLR = NRF_TIMER_INT_MASK(NRF_RTC_TIMER_CC_INT);
}

static uint32_t
hal_timer_read_bsptimer(struct nrf52_hal_timer *bsptimer)
{
    uint32_t low32;
    os_sr_t sr;
    uint32_t tcntr;
    NRF_RTC_Type *rtctimer;

    rtctimer = (NRF_RTC_Type *)bsptimer->tmr_reg;
    OS_ENTER_CRITICAL(sr);
    tcntr = bsptimer->tmr_cntr;
    low32 = rtctimer->COUNTER;
    if (rtctimer->EVENTS_OVRFLW) {
        tcntr += (1UL << 24);
        bsptimer->tmr_cntr = tcntr;
        low32 = rtctimer->COUNTER;
        rtctimer->EVENTS_OVRFLW = 0;
        NVIC_SetPendingIRQ(bsptimer->tmr_irq_num);
    }
    tcntr |= low32;
    OS_EXIT_CRITICAL(sr);

    return tcntr;
}

#if (MYNEWT_VAL(TIMER_0) || MYNEWT_VAL(TIMER_1) || MYNEWT_VAL(TIMER_2) || \
     MYNEWT_VAL(TIMER_3) || MYNEWT_VAL(TIMER_4) || MYNEWT_VAL(TIMER_5))
/**
 * hal timer chk queue
 *
 *
 * @param bsptimer
 */
static void
hal_timer_chk_queue(struct nrf52_hal_timer *bsptimer)
{
    int32_t delta;
    uint32_t tcntr;
    os_sr_t sr;
    struct hal_timer *timer;

    /* disable interrupts */
    OS_ENTER_CRITICAL(sr);
    while ((timer = TAILQ_FIRST(&bsptimer->hal_timer_q)) != NULL) {
        if (bsptimer->tmr_rtc) {
            tcntr = hal_timer_read_bsptimer(bsptimer);
            /*
             * If we are within 3 ticks of RTC, we wont be able to set compare.
             * Thus, we have to service this timer early.
             */
            delta = -3;
        } else {
            tcntr = nrf_read_timer_cntr(bsptimer->tmr_reg);
            delta = 0;
        }
        if ((int32_t)(tcntr - timer->expiry) >= delta) {
            TAILQ_REMOVE(&bsptimer->hal_timer_q, timer, link);
            timer->link.tqe_prev = NULL;
            timer->cb_func(timer->cb_arg);
        } else {
            break;
        }
    }

    /* Any timers left on queue? If so, we need to set OCMP */
    timer = TAILQ_FIRST(&bsptimer->hal_timer_q);
    if (timer) {
        nrf_timer_set_ocmp(bsptimer, timer->expiry);
    } else {
        if (bsptimer->tmr_rtc) {
            nrf_rtc_disable_ocmp((NRF_RTC_Type *)bsptimer->tmr_reg);
        } else {
            nrf_timer_disable_ocmp(bsptimer->tmr_reg);
        }
    }
    OS_EXIT_CRITICAL(sr);
}
#endif

/**
 * hal timer irq handler
 *
 * Generic HAL timer irq handler.
 *
 * @param tmr
 */
/**
 * hal timer irq handler
 *
 * This is the global timer interrupt routine.
 *
 */
#if (MYNEWT_VAL(TIMER_0) || MYNEWT_VAL(TIMER_1) || MYNEWT_VAL(TIMER_2) || \
     MYNEWT_VAL(TIMER_3) || MYNEWT_VAL(TIMER_4))

static void
hal_timer_irq_handler(struct nrf52_hal_timer *bsptimer)
{
    uint32_t compare;
    NRF_TIMER_Type *hwtimer;

    os_trace_enter_isr();

    /* Check interrupt source. If set, clear them */
    hwtimer = bsptimer->tmr_reg;
    compare = hwtimer->EVENTS_COMPARE[NRF_TIMER_CC_INT];
    if (compare) {
        hwtimer->EVENTS_COMPARE[NRF_TIMER_CC_INT] = 0;
    }

    /* XXX: make these stats? */
    /* Count # of timer isrs */
    ++bsptimer->timer_isrs;

    /*
     * NOTE: we dont check the 'compare' variable here due to how the timer
     * is implemented on this chip. There is no way to force an output
     * compare, so if we are late setting the output compare (i.e. the timer
     * counter is already passed the output compare value), we use the NVIC
     * to set a pending interrupt. This means that there will be no compare
     * flag set, so all we do is check to see if the compare interrupt is
     * enabled.
     */
    if (hwtimer->INTENCLR & NRF_TIMER_INT_MASK(NRF_TIMER_CC_INT)) {
        hal_timer_chk_queue(bsptimer);
        /* XXX: Recommended by nordic to make sure interrupts are cleared */
        compare = hwtimer->EVENTS_COMPARE[NRF_TIMER_CC_INT];
    }

    os_trace_exit_isr();
}
#endif

#if MYNEWT_VAL(TIMER_5)
static void
hal_rtc_timer_irq_handler(struct nrf52_hal_timer *bsptimer)
{
    uint32_t overflow;
    uint32_t compare;
    NRF_RTC_Type *rtctimer;

    /* Check interrupt source. If set, clear them */
    rtctimer = (NRF_RTC_Type *)bsptimer->tmr_reg;
    compare = rtctimer->EVENTS_COMPARE[NRF_RTC_TIMER_CC_INT];
    if (compare) {
       rtctimer->EVENTS_COMPARE[NRF_RTC_TIMER_CC_INT] = 0;
    }

    overflow = rtctimer->EVENTS_OVRFLW;
    if (overflow) {
        rtctimer->EVENTS_OVRFLW = 0;
        bsptimer->tmr_cntr += (1UL << 24);
    }

    /* Count # of timer isrs */
    ++bsptimer->timer_isrs;

    /*
     * NOTE: we dont check the 'compare' variable here due to how the timer
     * is implemented on this chip. There is no way to force an output
     * compare, so if we are late setting the output compare (i.e. the timer
     * counter is already passed the output compare value), we use the NVIC
     * to set a pending interrupt. This means that there will be no compare
     * flag set, so all we do is check to see if the compare interrupt is
     * enabled.
     */
    hal_timer_chk_queue(bsptimer);

    /* Recommended by nordic to make sure interrupts are cleared */
    compare = rtctimer->EVENTS_COMPARE[NRF_RTC_TIMER_CC_INT];
}
#endif

#if MYNEWT_VAL(TIMER_0)
void
nrf52_timer0_irq_handler(void)
{
    hal_timer_irq_handler(&nrf52_hal_timer0);
}
#endif

#if MYNEWT_VAL(TIMER_1)
void
nrf52_timer1_irq_handler(void)
{
    hal_timer_irq_handler(&nrf52_hal_timer1);
}
#endif

#if MYNEWT_VAL(TIMER_2)
void
nrf52_timer2_irq_handler(void)
{
    hal_timer_irq_handler(&nrf52_hal_timer2);
}
#endif

#if MYNEWT_VAL(TIMER_3)
void
nrf52_timer3_irq_handler(void)
{
    hal_timer_irq_handler(&nrf52_hal_timer3);
}
#endif

#if MYNEWT_VAL(TIMER_4)
void
nrf52_timer4_irq_handler(void)
{
    hal_timer_irq_handler(&nrf52_hal_timer4);
}
#endif

#if MYNEWT_VAL(TIMER_5)
void
nrf52_timer5_irq_handler(void)
{
    hal_rtc_timer_irq_handler(&nrf52_hal_timer5);
}
#endif

/**
 * hal timer init
 *
 * Initialize platform specific timer items
 *
 * @param timer_num     Timer number to initialize
 * @param cfg           Pointer to platform specific configuration
 *
 * @return int          0: success; error code otherwise
 */
int
hal_timer_init(int timer_num, void *cfg)
{
    int rc;
    uint8_t irq_num;
    struct nrf52_hal_timer *bsptimer;
    void *hwtimer;
    hal_timer_irq_handler_t irq_isr;

    NRF52_HAL_TIMER_RESOLVE(timer_num, bsptimer);

    /* If timer is enabled do not allow init */
    if (bsptimer->tmr_enabled) {
        rc = EINVAL;
        goto err;
    }

    switch (timer_num) {
#if MYNEWT_VAL(TIMER_5)
    case 5:
        irq_num = RTC0_IRQn;
        hwtimer = NRF_RTC0;
        irq_isr = nrf52_timer5_irq_handler;
        bsptimer->tmr_rtc = 1;
        break;
#endif
    default:
        hwtimer = NULL;
        break;
    }

    if (hwtimer == NULL) {
        rc = EINVAL;
        goto err;
    }

    bsptimer->tmr_reg = hwtimer;
    bsptimer->tmr_irq_num = irq_num;

    /* Disable IRQ, set priority and set vector in table */
    NVIC_DisableIRQ(irq_num);
#ifndef RIOT_VERSION
    NVIC_SetPriority(irq_num, (1 << __NVIC_PRIO_BITS) - 1);
#endif
#if MYNEWT
    NVIC_SetVector(irq_num, (uint32_t)irq_isr);
#else
    ble_npl_hw_set_isr(irq_num, irq_isr);
#endif

    return 0;

err:
    return rc;
}

/**
 * hal timer config
 *
 * Configure a timer to run at the desired frequency. This starts the timer.
 *
 * @param timer_num
 * @param freq_hz
 *
 * @return int
 */
int
hal_timer_config(int timer_num, uint32_t freq_hz)
{
    int rc;
    os_sr_t sr;
    struct nrf52_hal_timer *bsptimer;
#if MYNEWT_VAL(TIMER_5)
    NRF_RTC_Type *rtctimer;
#endif

    NRF52_HAL_TIMER_RESOLVE(timer_num, bsptimer);

#if MYNEWT_VAL(TIMER_5)
    if (timer_num == 5) {
        /* NOTE: we only allow the RTC frequency to be set at 32768 */
        if (bsptimer->tmr_enabled || (freq_hz != 32768) ||
            (bsptimer->tmr_reg == NULL)) {
            rc = EINVAL;
            goto err;
        }

        bsptimer->tmr_freq = freq_hz;
        bsptimer->tmr_enabled = 1;

        OS_ENTER_CRITICAL(sr);

        rtctimer = (NRF_RTC_Type *)bsptimer->tmr_reg;

        /* Stop the timer first */
        rtctimer->TASKS_STOP = 1;

        /* Always no prescaler */
        rtctimer->PRESCALER = 0;

        /* Clear overflow events and set overflow interrupt */
        rtctimer->EVENTS_OVRFLW = 0;
        rtctimer->INTENSET = RTC_INTENSET_OVRFLW_Msk;

        /* Start the timer */
        rtctimer->TASKS_START = 1;

        /* Set isr in vector table and enable interrupt */
        NVIC_EnableIRQ(bsptimer->tmr_irq_num);

        OS_EXIT_CRITICAL(sr);
        return 0;
    }
#endif

    assert(0);

    return 0;

err:
    return rc;
}

/**
 * hal timer deinit
 *
 * De-initialize a HW timer.
 *
 * @param timer_num
 *
 * @return int
 */
int
hal_timer_deinit(int timer_num)
{
    int rc;
    os_sr_t sr;
    struct nrf52_hal_timer *bsptimer;
    NRF_TIMER_Type *hwtimer;
    NRF_RTC_Type *rtctimer;

    rc = 0;
    NRF52_HAL_TIMER_RESOLVE(timer_num, bsptimer);

    OS_ENTER_CRITICAL(sr);
    if (bsptimer->tmr_rtc) {
        rtctimer = (NRF_RTC_Type *)bsptimer->tmr_reg;
        rtctimer->INTENCLR = NRF_TIMER_INT_MASK(NRF_RTC_TIMER_CC_INT);
        rtctimer->TASKS_STOP = 1;
    } else {
        hwtimer = (NRF_TIMER_Type *)bsptimer->tmr_reg;
        hwtimer->INTENCLR = NRF_TIMER_INT_MASK(NRF_TIMER_CC_INT);
        hwtimer->TASKS_STOP = 1;
    }
    bsptimer->tmr_enabled = 0;
    bsptimer->tmr_reg = NULL;
    OS_EXIT_CRITICAL(sr);

err:
    return rc;
}

/**
 * hal timer get resolution
 *
 * Get the resolution of the timer. This is the timer period, in nanoseconds
 *
 * @param timer_num
 *
 * @return uint32_t The
 */
uint32_t
hal_timer_get_resolution(int timer_num)
{
    int rc;
    uint32_t resolution;
    struct nrf52_hal_timer *bsptimer;

    NRF52_HAL_TIMER_RESOLVE(timer_num, bsptimer);

    resolution = 1000000000 / bsptimer->tmr_freq;
    return resolution;

err:
    rc = 0;
    return rc;
}

/**
 * hal timer read
 *
 * Returns the timer counter. NOTE: if the timer is a 16-bit timer, only
 * the lower 16 bits are valid. If the timer is a 64-bit timer, only the
 * low 32-bits are returned.
 *
 * @return uint32_t The timer counter register.
 */
uint32_t
hal_timer_read(int timer_num)
{
    int rc;
    uint32_t tcntr;
    struct nrf52_hal_timer *bsptimer;

    NRF52_HAL_TIMER_RESOLVE(timer_num, bsptimer);
    if (bsptimer->tmr_rtc) {
        tcntr = hal_timer_read_bsptimer(bsptimer);
    } else {
        tcntr = nrf_read_timer_cntr(bsptimer->tmr_reg);
    }

    return tcntr;

    /* Assert here since there is no invalid return code */
err:
    assert(0);
    rc = 0;
    return rc;
}

/**
 * hal timer delay
 *
 * Blocking delay for n ticks
 *
 * @param timer_num
 * @param ticks
 *
 * @return int 0 on success; error code otherwise.
 */
int
hal_timer_delay(int timer_num, uint32_t ticks)
{
    uint32_t until;

    until = hal_timer_read(timer_num) + ticks;
    while ((int32_t)(hal_timer_read(timer_num) - until) <= 0) {
        /* Loop here till finished */
    }

    return 0;
}

/**
 *
 * Initialize the HAL timer structure with the callback and the callback
 * argument. Also initializes the HW specific timer pointer.
 *
 * @param cb_func
 *
 * @return int
 */
int
hal_timer_set_cb(int timer_num, struct hal_timer *timer, hal_timer_cb cb_func,
                 void *arg)
{
    int rc;
    struct nrf52_hal_timer *bsptimer;

    NRF52_HAL_TIMER_RESOLVE(timer_num, bsptimer);

    timer->cb_func = cb_func;
    timer->cb_arg = arg;
    timer->link.tqe_prev = NULL;
    timer->bsp_timer = bsptimer;

    rc = 0;

err:
    return rc;
}

int
hal_timer_start(struct hal_timer *timer, uint32_t ticks)
{
    int rc;
    uint32_t tick;
    struct nrf52_hal_timer *bsptimer;

    /* Set the tick value at which the timer should expire */
    bsptimer = (struct nrf52_hal_timer *)timer->bsp_timer;
    if (bsptimer->tmr_rtc) {
        tick = hal_timer_read_bsptimer(bsptimer) + ticks;
    } else {
        tick = nrf_read_timer_cntr(bsptimer->tmr_reg) + ticks;
    }
    rc = hal_timer_start_at(timer, tick);
    return rc;
}

int
hal_timer_start_at(struct hal_timer *timer, uint32_t tick)
{
    os_sr_t sr;
    struct hal_timer *entry;
    struct nrf52_hal_timer *bsptimer;

    if ((timer == NULL) || (timer->link.tqe_prev != NULL) ||
        (timer->cb_func == NULL)) {
        return EINVAL;
    }
    bsptimer = (struct nrf52_hal_timer *)timer->bsp_timer;
    timer->expiry = tick;

    OS_ENTER_CRITICAL(sr);

    if (TAILQ_EMPTY(&bsptimer->hal_timer_q)) {
        TAILQ_INSERT_HEAD(&bsptimer->hal_timer_q, timer, link);
    } else {
        TAILQ_FOREACH(entry, &bsptimer->hal_timer_q, link) {
            if ((int32_t)(timer->expiry - entry->expiry) < 0) {
                TAILQ_INSERT_BEFORE(entry, timer, link);
                break;
            }
        }
        if (!entry) {
            TAILQ_INSERT_TAIL(&bsptimer->hal_timer_q, timer, link);
        }
    }

    /* If this is the head, we need to set new OCMP */
    if (timer == TAILQ_FIRST(&bsptimer->hal_timer_q)) {
        nrf_timer_set_ocmp(bsptimer, timer->expiry);
    }

    OS_EXIT_CRITICAL(sr);

    return 0;
}

/**
 * hal timer stop
 *
 * Stop a timer.
 *
 * @param timer
 *
 * @return int
 */
int
hal_timer_stop(struct hal_timer *timer)
{
    os_sr_t sr;
    int reset_ocmp;
    struct hal_timer *entry;
    struct nrf52_hal_timer *bsptimer;

    if (timer == NULL) {
        return EINVAL;
    }

   bsptimer = (struct nrf52_hal_timer *)timer->bsp_timer;

    OS_ENTER_CRITICAL(sr);

    if (timer->link.tqe_prev != NULL) {
        reset_ocmp = 0;
        if (timer == TAILQ_FIRST(&bsptimer->hal_timer_q)) {
            /* If first on queue, we will need to reset OCMP */
            entry = TAILQ_NEXT(timer, link);
            reset_ocmp = 1;
        }
        TAILQ_REMOVE(&bsptimer->hal_timer_q, timer, link);
        timer->link.tqe_prev = NULL;
        if (reset_ocmp) {
            if (entry) {
                nrf_timer_set_ocmp((struct nrf52_hal_timer *)entry->bsp_timer,
                                   entry->expiry);
            } else {
                if (bsptimer->tmr_rtc) {
                    nrf_rtc_disable_ocmp((NRF_RTC_Type *)bsptimer->tmr_reg);
                } else {
                    nrf_timer_disable_ocmp(bsptimer->tmr_reg);
                }
            }
        }
    }

    OS_EXIT_CRITICAL(sr);

    return 0;
}
