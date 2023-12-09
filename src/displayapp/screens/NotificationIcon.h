#pragma once
#include "components/ble/NotificationManager.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class NotificationIcon {
      public:
        static const char* GetIcon(bool newNotificationAvailable);
        static const char* GetCategoryIcon(Pinetime::Controllers::NotificationManager::Categories category);
      };
    }
  }
}