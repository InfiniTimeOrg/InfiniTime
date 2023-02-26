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

      class SettingBluetooth : public Screen {
      public:
        SettingBluetooth(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingBluetooth() override;

      private:
        DisplayApp* app;
        CheckboxList checkboxList;
      };
    }
  }
}
