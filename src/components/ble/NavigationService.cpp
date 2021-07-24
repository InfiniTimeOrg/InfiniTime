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

#include "NavigationService.h"

#include "systemtask/SystemTask.h"

int NAVCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto navService = static_cast<Pinetime::Controllers::NavigationService*>(arg);
  return navService->OnCommand(conn_handle, attr_handle, ctxt);
}

Pinetime::Controllers::NavigationService::NavigationService(Pinetime::System::SystemTask& system) : m_system(system) {
  navFlagCharUuid.value[12] = navFlagCharId[0];
  navFlagCharUuid.value[13] = navFlagCharId[1];

  navNarrativeCharUuid.value[12] = navNarrativeCharId[0];
  navNarrativeCharUuid.value[13] = navNarrativeCharId[1];

  navManDistCharUuid.value[12] = navManDistCharId[0];
  navManDistCharUuid.value[13] = navManDistCharId[1];

  navProgressCharUuid.value[12] = navProgressCharId[0];
  navProgressCharUuid.value[13] = navProgressCharId[1];

  characteristicDefinition[0] = {
    .uuid = (ble_uuid_t*) (&navFlagCharUuid), .access_cb = NAVCallback, .arg = this, .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ};

  characteristicDefinition[1] = {.uuid = (ble_uuid_t*) (&navNarrativeCharUuid),
                                 .access_cb = NAVCallback,
                                 .arg = this,
                                 .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ};
  characteristicDefinition[2] = {.uuid = (ble_uuid_t*) (&navManDistCharUuid),
                                 .access_cb = NAVCallback,
                                 .arg = this,
                                 .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ};
  characteristicDefinition[3] = {.uuid = (ble_uuid_t*) (&navProgressCharUuid),
                                 .access_cb = NAVCallback,
                                 .arg = this,
                                 .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ};

  characteristicDefinition[4] = {0};

  serviceDefinition[0] = {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = (ble_uuid_t*) &navUuid, .characteristics = characteristicDefinition};
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

int Pinetime::Controllers::NavigationService::OnCommand(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt) {

  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    size_t notifSize = OS_MBUF_PKTLEN(ctxt->om);
    uint8_t data[notifSize + 1];
    data[notifSize] = '\0';
    os_mbuf_copydata(ctxt->om, 0, notifSize, data);
    char* s = (char*) &data[0];
    if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t*) &navFlagCharUuid) == 0) {
      m_flag = s;
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t*) &navNarrativeCharUuid) == 0) {
      m_narrative = s;
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t*) &navManDistCharUuid) == 0) {
      m_manDist = s;
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t*) &navProgressCharUuid) == 0) {
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
