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

      class SettingWeatherFormat : public Screen {
      public:
        explicit SettingWeatherFormat(Pinetime::Controllers::Settings& settingsController);
        ~SettingWeatherFormat() override;

      private:
        CheckboxList checkboxList;
      };
    }
  }
}
