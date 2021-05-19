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

#include <nuttx/config.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include "nimble/nimble_npl.h"

static struct ble_npl_eventq dflt_evq =
{
  .mq = (mqd_t)-1
};

int qnum = 0;

struct ble_npl_eventq *
ble_npl_eventq_dflt_get(void)
{
    return &dflt_evq;
}

void
ble_npl_eventq_init(struct ble_npl_eventq *evq)
{
    char path[PATH_MAX];
    struct mq_attr attr;
    attr.mq_maxmsg = 16;
    attr.mq_msgsize = sizeof(struct ble_npl_event*);
    attr.mq_flags = 0;

    qnum++;
    sprintf(path, "/nimbleq%i", qnum);
    evq->mq = mq_open(path, O_CREAT | O_RDWR, 0, &attr);
    DEBUGASSERT((intptr_t)evq->mq >= 0);
}

bool
ble_npl_eventq_is_empty(struct ble_npl_eventq *evq)
{
    struct mq_attr attr;
    mq_getattr(evq->mq, &attr);

    return (attr.mq_curmsgs == 0);
}

int
ble_npl_eventq_inited(const struct ble_npl_eventq *evq)
{
    return (evq->mq != ((mqd_t)-1));
}

void
ble_npl_eventq_put(struct ble_npl_eventq *evq, struct ble_npl_event *ev)
{
    int ret;

    if (ev->ev_queued)
      {
        return;
      }

    ev->ev_queued = 1;
    ret = mq_send(evq->mq, (const char*)&ev, sizeof(ev), 0);

    DEBUGASSERT(ret == 0);
    UNUSED(ret);
}

struct ble_npl_event *
ble_npl_eventq_get(struct ble_npl_eventq *evq,
                   ble_npl_time_t tmo)
{
    struct ble_npl_event *ev = NULL;

    if (tmo == BLE_NPL_TIME_FOREVER)
      {
        do
          {
            mq_receive(evq->mq, (char*)&ev, sizeof(ev), NULL);
          } while (ev && ev->ev_queued == 0);
      }
    else
      {
        struct timespec now, t;

        clock_gettime(CLOCK_REALTIME, &now);

        t.tv_sec += tmo / USEC_PER_SEC;
        t.tv_nsec = (tmo - t.tv_sec) * NSEC_PER_MSEC;
        clock_timespec_add(&now, &t, &t);

        do
          {
            mq_timedreceive(evq->mq, (char*)&ev, sizeof(ev), NULL, &t);
          } while (ev && ev->ev_queued == 0);
      }

    if (ev) {
        ev->ev_queued = 0;
    }

    return ev;
}

void
ble_npl_eventq_run(struct ble_npl_eventq *evq)
{
    struct ble_npl_event *ev;

    ev = ble_npl_eventq_get(evq, BLE_NPL_TIME_FOREVER);
    ble_npl_event_run(ev);
}


// ========================================================================
//                         Event Implementation
// ========================================================================

void
ble_npl_event_init(struct ble_npl_event *ev, ble_npl_event_fn *fn,
                   void *arg)
{
    memset(ev, 0, sizeof(*ev));
    ev->ev_cb = fn;
    ev->ev_arg = arg;
}

bool
ble_npl_event_is_queued(struct ble_npl_event *ev)
{
    return ev->ev_queued;
}

void *
ble_npl_event_get_arg(struct ble_npl_event *ev)
{
    return ev->ev_arg;
}

void
ble_npl_event_set_arg(struct ble_npl_event *ev, void *arg)
{
    ev->ev_arg = arg;
}

void
ble_npl_event_run(struct ble_npl_event *ev)
{
    assert(ev->ev_cb != NULL);

    ev->ev_cb(ev);
}

void
ble_npl_eventq_remove(struct ble_npl_eventq *evq, struct ble_npl_event *ev)
{
    /* we simply mark the event as unqueued. we will ignore these elements
     * when receiving from the queue */

    ev->ev_queued = 0;
}
