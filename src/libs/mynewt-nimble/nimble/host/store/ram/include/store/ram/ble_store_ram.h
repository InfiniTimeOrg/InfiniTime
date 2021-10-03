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

/* This package has been deprecated and you should
 * use the store/config package. For a RAM-only BLE store,
 * use store/config and set BLE_STORE_CONFIG_PERSIST to 0.
 */

#ifndef H_BLE_STORE_RAM_
#define H_BLE_STORE_RAM_

#ifdef __cplusplus
extern "C" {
#endif

union ble_store_key;
union ble_store_value;

int ble_store_ram_read(int obj_type, const union ble_store_key *key,
                       union ble_store_value *value);
int ble_store_ram_write(int obj_type, const union ble_store_value *val);
int ble_store_ram_delete(int obj_type, const union ble_store_key *key);

#ifdef __cplusplus
}
#endif

#endif
