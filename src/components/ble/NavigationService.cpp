/*  Copyright (C) 2021  Adam Pigg

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

#include "components/ble/NavigationService.h"

#include "systemtask/SystemTask.h"

namespace {
  // 0001yyxx-78fc-48fe-8e23-433b3a1942d0
  constexpr ble_uuid128_t CharUuid(uint8_t x, uint8_t y) {
    return ble_uuid128_t {.u = {.type = BLE_UUID_TYPE_128},
                          .value = {0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, x, y, 0x01, 0x00}};
  }

  // 00010000-78fc-48fe-8e23-433b3a1942d0
  constexpr ble_uuid128_t BaseUuid() {
    return CharUuid(0x00, 0x00);
  }

  constexpr ble_uuid128_t navUuid {BaseUuid()};

  constexpr ble_uuid128_t navFlagCharUuid {CharUuid(0x01, 0x00)};
  constexpr ble_uuid128_t navNarrativeCharUuid {CharUuid(0x02, 0x00)};
  constexpr ble_uuid128_t navManDistCharUuid {CharUuid(0x03, 0x00)};
  constexpr ble_uuid128_t navProgressCharUuid {CharUuid(0x04, 0x00)};

  int NAVCallback(uint16_t /*conn_handle*/, uint16_t /*attr_handle*/, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    auto* navService = static_cast<Pinetime::Controllers::NavigationService*>(arg);
    return navService->OnCommand(ctxt);
  }
} // namespace

Pinetime::Controllers::NavigationService::NavigationService(Pinetime::System::SystemTask& system) : m_system(system) {
  characteristicDefinition[0] = {.uuid = &navFlagCharUuid.u,
                                 .access_cb = NAVCallback,
                                 .arg = this,
                                 .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ};

  characteristicDefinition[1] = {.uuid = &navNarrativeCharUuid.u,
                                 .access_cb = NAVCallback,
                                 .arg = this,
                                 .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ};
  characteristicDefinition[2] = {.uuid = &navManDistCharUuid.u,
                                 .access_cb = NAVCallback,
                                 .arg = this,
                                 .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ};
  characteristicDefinition[3] = {.uuid = &navProgressCharUuid.u,
                                 .access_cb = NAVCallback,
                                 .arg = this,
                                 .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ};

  characteristicDefinition[4] = {0};

  serviceDefinition[0] = {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &navUuid.u, .characteristics = characteristicDefinition};
  serviceDefinition[1] = {0};

  m_progress = 0;
}

void Pinetime::Controllers::NavigationService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int Pinetime::Controllers::NavigationService::OnCommand(struct ble_gatt_access_ctxt* ctxt) {

  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    size_t notifSize = OS_MBUF_PKTLEN(ctxt->om);
    uint8_t data[notifSize + 1];
    data[notifSize] = '\0';
    os_mbuf_copydata(ctxt->om, 0, notifSize, data);
    char* s = (char*) &data[0];
    if (ble_uuid_cmp(ctxt->chr->uuid, &navFlagCharUuid.u) == 0) {
      m_flag = s;
    } else if (ble_uuid_cmp(ctxt->chr->uuid, &navNarrativeCharUuid.u) == 0) {
      m_narrative = s;
    } else if (ble_uuid_cmp(ctxt->chr->uuid, &navManDistCharUuid.u) == 0) {
      m_manDist = s;
    } else if (ble_uuid_cmp(ctxt->chr->uuid, &navProgressCharUuid.u) == 0) {
      m_progress = data[0];
    }
  }
  return 0;
}

std::string Pinetime::Controllers::NavigationService::getFlag() {
  return m_flag;
}

std::string Pinetime::Controllers::NavigationService::getNarrative() {
  return m_narrative;
}

std::string Pinetime::Controllers::NavigationService::getManDist() {
  return m_manDist;
}

int Pinetime::Controllers::NavigationService::getProgress() {
  return m_progress;
}
