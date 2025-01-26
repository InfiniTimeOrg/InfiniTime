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

      class SettingNotifVibration : public Screen {
      public:
        explicit SettingNotifVibration(Pinetime::Controllers::Settings& settingsController,
                                       Pinetime::Controllers::MotorController& motorController);
        ~SettingNotifVibration() override;

      private:
        CheckboxList checkboxList;
      };
    }
  }
}