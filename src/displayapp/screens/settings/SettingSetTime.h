#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "displayapp/widgets/Counter.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SettingSetTime : public Screen {
      public:
        SettingSetTime(DisplayApp* app,
                       Pinetime::Controllers::DateTime& dateTimeController,
                       Pinetime::Controllers::Settings& settingsController);
        ~SettingSetTime() override;

        void SetTime();
        void UpdateScreen();

      private:
        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;

        lv_obj_t* lblampm;
        lv_obj_t* btnSetTime;
        lv_obj_t* lblSetTime;
        Widgets::Counter hourCounter = Widgets::Counter(0, 23, jetbrains_mono_42);
        Widgets::Counter minuteCounter = Widgets::Counter(0, 59, jetbrains_mono_42);
      };
    }
  }
}
