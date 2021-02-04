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

#ifndef _BLE_HCI_TRANS_H4_H_
#define _BLE_HCI_TRANS_H4_H_

#include <stdint.h>

#define BLE_HCI_TRANS_H4_PKT_TYPE_NONE      0x00
#define BLE_HCI_TRANS_H4_PKT_TYPE_CMD       0x01
#define BLE_HCI_TRANS_H4_PKT_TYPE_ACL       0x02
#define BLE_HCI_TRANS_H4_PKT_TYPE_EVT       0x04

struct ble_hci_trans_h4_rx_state {
    uint8_t state;
    uint8_t pkt_type;
    uint8_t min_len;
    uint16_t len;
    uint16_t expected_len;
    uint8_t hdr[4];
    union {
        uint8_t *buf;
        struct os_mbuf *om;
    };
};

typedef int (ble_hci_trans_h4_frame_cb)(uint8_t pkt_type, void *data);

int ble_hci_trans_h4_rx(struct ble_hci_trans_h4_rx_state *rxs,
                        const uint8_t *buf, uint16_t len,
                        ble_hci_trans_h4_frame_cb *frame_cb);

#endif /* _BLE_HCI_TRANS_H4_H_ */
