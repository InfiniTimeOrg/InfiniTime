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

      class SettingBluetoothMain : public Screen {
      public:
        SettingBluetoothMain(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingBluetoothMain() override;

      private:
        DisplayApp* app;
        Pinetime::Controllers::Settings& settings;
        CheckboxList checkboxList;
      };
    }
  }
}
