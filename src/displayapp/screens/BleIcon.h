#pragma once

#include "components/ble/BleController.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class BleIcon {
      public:
        static const char* GetIcon(bool isConnected);
      };
    }
  }
}
