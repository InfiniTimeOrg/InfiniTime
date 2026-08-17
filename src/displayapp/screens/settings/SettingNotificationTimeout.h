#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingNotificationTimeout : public Screen {
      public:
        SettingNotificationTimeout(Pinetime::Controllers::Settings& settingsController);
        ~SettingNotificationTimeout() override;

      private:
        CheckboxList checkboxList;
      };
    }
  }
}
