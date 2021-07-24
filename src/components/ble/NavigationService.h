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
#pragma once

#include <cstdint>
#include <string>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <host/ble_uuid.h>
#undef max
#undef min

// 00010000-78fc-48fe-8e23-433b3a1942d0
#define NAVIGATION_SERVICE_UUID_BASE                                                                                                       \
  { 0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, 0x00, 0x00, 0x00, 0x00 }

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {

    class NavigationService {
    public:
      explicit NavigationService(Pinetime::System::SystemTask& system);

      void Init();

      int OnCommand(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt);

      std::string getFlag();

      std::string getNarrative();

      std::string getManDist();

      int getProgress();

    private:
      static constexpr uint8_t navId[2] = {0x01, 0x00};
      static constexpr uint8_t navFlagCharId[2] = {0x01, 0x00};
      static constexpr uint8_t navNarrativeCharId[2] = {0x02, 0x00};
      static constexpr uint8_t navManDistCharId[2] = {0x03, 0x00};
      static constexpr uint8_t navProgressCharId[2] = {0x04, 0x00};

      ble_uuid128_t navUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = NAVIGATION_SERVICE_UUID_BASE};

      ble_uuid128_t navFlagCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = NAVIGATION_SERVICE_UUID_BASE};
      ble_uuid128_t navNarrativeCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = NAVIGATION_SERVICE_UUID_BASE};
      ble_uuid128_t navManDistCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = NAVIGATION_SERVICE_UUID_BASE};
      ble_uuid128_t navProgressCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = NAVIGATION_SERVICE_UUID_BASE};

      struct ble_gatt_chr_def characteristicDefinition[5];
      struct ble_gatt_svc_def serviceDefinition[2];

      std::string m_flag;
      std::string m_narrative;
      std::string m_manDist;
      int m_progress;

      Pinetime::System::SystemTask& m_system;
    };
  }
}
