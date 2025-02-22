#pragma once

#include "components/alarm/AlarmController.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class AlarmIcon {
      public:
        static const char* GetIcon(bool isSet);
      };
    }
  }
}
