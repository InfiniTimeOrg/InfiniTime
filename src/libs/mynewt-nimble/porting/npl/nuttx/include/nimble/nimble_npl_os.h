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

#ifndef _NIMBLE_NPL_OS_H_
#define _NIMBLE_NPL_OS_H_

#include <nuttx/config.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "os_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_NPL_OS_ALIGNMENT    4

#define BLE_NPL_TIME_FOREVER    INT32_MAX

#define SYSINIT_PANIC_MSG(msg) { fprintf(stderr, "%s\n", msg); abort(); }

#define SYSINIT_PANIC_ASSERT_MSG(rc, msg) do \
{                                            \
    if (!(rc)) {                             \
        SYSINIT_PANIC_MSG(msg);              \
    }                                        \
} while (0)

#ifdef __cplusplus
}
#endif

/* Define some variables since nimBLE is designed to be built ignoring
 * undefined macros and we do not ignore warnings on NuttX.
 *
 * Note: MYNEWT will give undefined warning, but some parts of the code
 * interpret !defined(MYNEWT) as MYNEWT=0, so we shouldn't define it to zero
 * either.
 *
 * Note 2: default_RNG_defined could probably set to 1 but this requires
 * testing
 */

#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_PRIVACY 0
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_EXT_ADV 0
#define MYNEWT_VAL_BLE_LL_DTM_EXTENSIONS 0
#define MYNEWT_VAL_BLE_CONTROLLER 0
#define default_RNG_defined 0
#define BLETEST_THROUGHPUT_TEST 0
#define MYNEWT_VAL_TRNG 0
#define MYNEWT_VAL_SELFTEST 0

#endif  /* _NPL_H_ */
