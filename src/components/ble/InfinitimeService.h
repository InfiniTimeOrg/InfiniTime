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
#pragma once

#include <cstdint>
#include <string>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <host/ble_uuid.h>
#undef max
#undef min

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    class InfinitimeService {
    public:
      explicit InfinitimeService(Pinetime::System::SystemTask& system);

      void Init();

      int OnCommand(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt);

      void event(char event);

      bool isPhoneFinding() const;

      static const char EVENT_PHONE_FIND_STOP = 0x00;
      static const char EVENT_PHONE_FIND_START = 0x01;

      enum FindPhone { FindPhoneRunning = 0x00, FindPhoneStopped = 0x01 };

    private:
      struct ble_gatt_chr_def characteristicDefinition[2];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t phoneFindingHandle {};
      bool m_isPhoneFinding {false};

      Pinetime::System::SystemTask& m_system;
    };
  }
}
