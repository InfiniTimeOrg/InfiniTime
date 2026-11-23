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
      class SettingClockFormat : public Screen {
      public:
        SettingClockFormat(Pinetime::Controllers::Settings& settingsController);
        ~SettingClockFormat() override;

      private:
        CheckboxList clockCheckboxList;
      };
    }
  }
}
