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

#ifndef _BLE_RF_PRIV_H_
#define _BLE_RF_PRIV_H_

void ble_rf_init(void);
void ble_rf_enable(void);
void ble_rf_stop(void);
void ble_rf_disable(void);
bool ble_rf_is_enabled(void);
void ble_rf_configure(void);

void ble_rf_calibrate(void);

void ble_rf_setup_tx(uint8_t rf_chan, uint8_t mode);
void ble_rf_setup_rx(uint8_t rf_chan, uint8_t mode);

void ble_rf_set_tx_power(int dbm);
int8_t ble_rf_get_rssi(void);

#endif /* _BLE_RF_PRIV_H_ */
