#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "components/motor/MotorController.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingChimeVibration : public Screen {
      public:
        explicit SettingChimeVibration(Pinetime::Controllers::Settings& settingsController,
                                       Pinetime::Controllers::MotorController& motorController);
        ~SettingChimeVibration() override;

      private:
        CheckboxList checkboxList;
      };
    }
  }
}