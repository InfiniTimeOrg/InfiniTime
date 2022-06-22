/*  Copyright (C) 2020-2022 JF, Adam Pigg, Avamander, devnoname120

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "components/ble/InfinitimeService.h"
#include "systemtask/SystemTask.h"
#include <cstring>

namespace {
  // 0001yyxx-b5e2-40c4-b1d5-2c5c48529c84
  constexpr ble_uuid128_t CharUuid(uint8_t x, uint8_t y) {
    return ble_uuid128_t {.u = {.type = BLE_UUID_TYPE_128},
                          .value = {0x84, 0x9c, 0x52, 0x48, 0x5c, 0x2c, 0xd5, 0xb1, 0xc4, 0x40, 0xe2, 0xb5, x, y, 0x01, 0x00}};
  }

  // 00010000-b5e2-40c4-b1d5-2c5c48529c84
  constexpr ble_uuid128_t BaseUuid() {
    return CharUuid(0x00, 0x00);
  }

  constexpr ble_uuid128_t infUuid {BaseUuid()};
  constexpr ble_uuid128_t infFindPhoneCharUuid {CharUuid(0x01, 0x00)};

  constexpr size_t MaxNotifSize {1024};

  int InfinitimeCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    return static_cast<Pinetime::Controllers::InfinitimeService*>(arg)->OnCommand(conn_handle, attr_handle, ctxt);
  }
}

Pinetime::Controllers::InfinitimeService::InfinitimeService(Pinetime::System::SystemTask& system) : m_system(system) {
  characteristicDefinition[0] = {.uuid = &infFindPhoneCharUuid.u,
                                 .access_cb = InfinitimeCallback,
                                 .arg = this,
                                 .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ,
                                 .val_handle = &phoneFindingHandle};
  characteristicDefinition[1] = {0};

  serviceDefinition[0] = {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &infUuid.u, .characteristics = characteristicDefinition};
  serviceDefinition[1] = {0};
}

void Pinetime::Controllers::InfinitimeService::Init() {
  uint8_t res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int Pinetime::Controllers::InfinitimeService::OnCommand(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt) {
  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    size_t notifSize = OS_MBUF_PKTLEN(ctxt->om);
    size_t bufferSize = notifSize;
    bool isNotifTruncated = false;

    if (notifSize > MaxNotifSize) {
      bufferSize = MaxNotifSize;
      isNotifTruncated = true;
    }

    char data[bufferSize + 1];
    os_mbuf_copydata(ctxt->om, 0, bufferSize, data);

    if (ble_uuid_cmp(ctxt->chr->uuid, &infFindPhoneCharUuid.u) == 0) {
      m_isPhoneFinding = static_cast<bool>(data[0]);
    }
  }
  return 0;
}

bool Pinetime::Controllers::InfinitimeService::isPhoneFinding() const {
  return m_isPhoneFinding;
}

void Pinetime::Controllers::InfinitimeService::event(char event) {
  auto* om = ble_hs_mbuf_from_flat(&event, 1);

  uint16_t connectionHandle = m_system.nimble().connHandle();

  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }

  ble_gattc_notify_custom(connectionHandle, phoneFindingHandle, om);
}
