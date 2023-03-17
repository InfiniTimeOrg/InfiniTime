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

      class SettingTimeFormat : public Screen {
      public:
        SettingTimeFormat(Pinetime::Controllers::Settings& settingsController);
        ~SettingTimeFormat() override;

      private:
        CheckboxList checkboxList;
      };
    }
  }
}
