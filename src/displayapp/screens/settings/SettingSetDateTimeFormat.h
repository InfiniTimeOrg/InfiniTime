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

      private:
        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;
        Pinetime::Applications::Screens::SettingSetDateTime& settingSetDateTime;

        lv_obj_t* btnSetTime;
        lv_obj_t* lblSetTime;
      };
    }
  }
}
