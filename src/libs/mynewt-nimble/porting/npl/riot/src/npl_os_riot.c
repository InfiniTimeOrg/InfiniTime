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

#include <stddef.h>
#include "nimble/nimble_npl.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

volatile int ble_npl_in_critical = 0;

static void
_callout_fire(void *arg)
{
    struct ble_npl_callout *co = (struct ble_npl_callout *)arg;
    event_post(co->q, &co->e.e.super);
}

ble_npl_error_t
ble_npl_sem_pend(struct ble_npl_sem *sem, ble_npl_time_t timeout)
{
    int rc = sema_wait_timed_ztimer(&sem->sem, ZTIMER_MSEC, timeout);
    return (rc == 0) ? BLE_NPL_OK : BLE_NPL_ENOENT;
}

void
ble_npl_callout_init(struct ble_npl_callout *c, struct ble_npl_eventq *evq,
                     ble_npl_event_fn *ev_cb, void *ev_arg)
{
    c->timer.arg = (void *)c;
    c->timer.callback = _callout_fire;
    c->q = &evq->q;
    ble_npl_event_init(&c->e, ev_cb, ev_arg);
}

ble_npl_error_t
ble_npl_callout_reset(struct ble_npl_callout *c, ble_npl_time_t ticks)
{
    /* Use critical section to ensure matching target_us and ztimer value. */
    uint32_t crit_state = ble_npl_hw_enter_critical();
    c->ticks = ztimer_now(ZTIMER_MSEC) + ticks;
    ztimer_set(ZTIMER_MSEC, &c->timer, ticks);
    ble_npl_hw_exit_critical(crit_state);
    return BLE_NPL_OK;
}

ble_npl_time_t
ble_npl_callout_remaining_ticks(struct ble_npl_callout *co,
                                ble_npl_time_t time)
{
    ztimer_now_t now = ztimer_now(ZTIMER_MSEC);
    return (ble_npl_time_t)(co->ticks - now);
}
