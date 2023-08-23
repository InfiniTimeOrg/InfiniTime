#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/datetime/DateTimeController.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/Counter.h"
#include "displayapp/widgets/DotIndicator.h"
#include "displayapp/screens/settings/SettingSetDateTime.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SettingSetDateTimeFormat : public Screen {
      public:
        SettingSetDateTimeFormat(Pinetime::Controllers::DateTime& dateTimeController,
                                 Pinetime::Controllers::Settings& settingsController,
                                 Pinetime::Applications::Screens::SettingSetDateTime& settingSetDateTime);
        ~SettingSetDateTimeFormat() override;

        void HandleButtonPress(lv_obj_t* buttonPressed);
        void SetSelectedColors();

      private:
        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;
        Pinetime::Applications::Screens::SettingSetDateTime& settingSetDateTime;

        lv_obj_t* btnTime12hr;
        lv_obj_t* btnTime24hr;

        lv_obj_t* btnDateDmy;
        lv_obj_t* btnDateMdy;
      };
    }
  }
}
