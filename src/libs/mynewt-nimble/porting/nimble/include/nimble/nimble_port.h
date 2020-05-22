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

#ifndef _NIMBLE_PORT_H
#define _NIMBLE_PORT_H

#include "nimble/nimble_npl.h"

#ifdef __cplusplus
extern "C" {
#endif

void nimble_port_init(void);

void nimble_port_run(void);

struct ble_npl_eventq *nimble_port_get_dflt_eventq(void);

#if NIMBLE_CFG_CONTROLLER
void nimble_port_ll_task_func(void *arg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _NIMBLE_PORT_H */
