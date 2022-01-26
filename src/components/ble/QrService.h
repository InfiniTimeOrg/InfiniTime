#pragma once

#include <cstdint>
#include <string>
#include <array>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <host/ble_uuid.h>
#undef max
#undef min

#define MAXLISTITEMS 4

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    class QrService {
    public:
      struct QrInfo {
        std::string name;
        std::string text;
        bool operator==(const QrInfo& rhs) const {
          return (rhs.name == name && rhs.text == text);
        }
      };

      explicit QrService(Pinetime::System::SystemTask& system);

      void Init();

      int OnCommand(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt);

      std::array<QrInfo, 4> getQrList();

    private:
      Pinetime::System::SystemTask& system;

      struct ble_gatt_chr_def characteristicDefinition[2];
      struct ble_gatt_svc_def serviceDefinition[2];

      std::array<QrInfo, 4> qrList;
    };
  }
}
