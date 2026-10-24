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

      class SettingOTA : public Screen {
      public:
        SettingOTA(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingOTA() override;

      private:
        DisplayApp* app;
        Pinetime::Controllers::Settings& settingsController;
        CheckboxList checkboxList;
      };
    }
  }
}
