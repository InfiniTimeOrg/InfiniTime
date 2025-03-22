#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/datetime/DateTimeController.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/Counter.h"
#include "displayapp/widgets/DotIndicator.h"
#include "displayapp/screens/settings/SettingSetDateTime.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SettingSetDate : public Screen {
      public:
        SettingSetDate(Pinetime::Controllers::DateTime& dateTimeController,
                       Pinetime::Applications::Screens::SettingSetDateTime& settingSetDateTime);
        ~SettingSetDate() override;

        void HandleButtonPress();
        void CheckDay();

      private:
        Controllers::DateTime& dateTimeController;
        Pinetime::Applications::Screens::SettingSetDateTime& settingSetDateTime;

        lv_obj_t* btnSetTime;
        lv_obj_t* lblSetTime;

        Widgets::Counter dayCounter = Widgets::Counter(1, 31, *Fonts::small);
        Widgets::Counter monthCounter = Widgets::Counter(1, 12, *Fonts::small);
        Widgets::Counter yearCounter = Widgets::Counter(1970, 9999, *Fonts::small);
      };
    }
  }
}
