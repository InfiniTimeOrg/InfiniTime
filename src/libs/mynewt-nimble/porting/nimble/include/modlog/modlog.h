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

#ifndef H_MODLOG_
#define H_MODLOG_

#include <stdio.h>

#include "log_common/log_common.h"
#include "log/log.h"

#define MODLOG_MODULE_DFLT 255

#if MYNEWT_VAL(LOG_LEVEL) <= LOG_LEVEL_DEBUG || defined __DOXYGEN__
#define MODLOG_DEBUG(ml_mod_, ml_msg_, ...) \
    printf((ml_msg_), ##__VA_ARGS__)
#else
#define MODLOG_DEBUG(ml_mod_, ...) IGNORE(__VA_ARGS__)
#endif

#if MYNEWT_VAL(LOG_LEVEL) <= LOG_LEVEL_INFO || defined __DOXYGEN__
#define MODLOG_INFO(ml_mod_, ml_msg_, ...) \
    printf((ml_msg_), ##__VA_ARGS__)
#else
#define MODLOG_INFO(ml_mod_, ...) IGNORE(__VA_ARGS__)
#endif

#if MYNEWT_VAL(LOG_LEVEL) <= LOG_LEVEL_WARN || defined __DOXYGEN__
#define MODLOG_WARN(ml_mod_, ml_msg_, ...) \
    printf((ml_msg_), ##__VA_ARGS__)
#else
#define MODLOG_WARN(ml_mod_, ...) IGNORE(__VA_ARGS__)
#endif

#if MYNEWT_VAL(LOG_LEVEL) <= LOG_LEVEL_ERROR || defined __DOXYGEN__
#define MODLOG_ERROR(ml_mod_, ml_msg_, ...) \
    printf((ml_msg_), ##__VA_ARGS__)
#else
#define MODLOG_ERROR(ml_mod_, ...) IGNORE(__VA_ARGS__)
#endif

#if MYNEWT_VAL(LOG_LEVEL) <= LOG_LEVEL_CRITICAL || defined __DOXYGEN__
#define MODLOG_CRITICAL(ml_mod_, ml_msg_, ...) \
    printf((ml_msg_), ##__VA_ARGS__)
#else
#define MODLOG_CRITICAL(ml_mod_, ...) IGNORE(__VA_ARGS__)
#endif

#define MODLOG(ml_lvl_, ml_mod_, ...) \
    MODLOG_ ## ml_lvl_((ml_mod_), __VA_ARGS__)

#endif
