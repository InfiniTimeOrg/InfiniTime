#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include "displayapp/screens/Screen.h"
#include "displayapp/DisplayApp.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "components/motor/MotorController.h"
#include "components/motion/MotionController.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class WatchFaceFuzzy : public Screen {
      public:
        WatchFaceFuzzy(DisplayApp* app,
                       Controllers::DateTime& dateTimeController,
                       Controllers::Settings& settingsController,
                       Controllers::MotorController& motorController,
                       Controllers::MotionController& motionController);
        ~WatchFaceFuzzy() override;

        void Refresh() override;

      private:
        const char* timeAccent = "ffffff";
        static const char* timeSectors[12];
        static const char* hourNames[12];
        bool shaking = true;
        bool should_digital;

        lv_obj_t* backgroundLabel;
        lv_obj_t* timeLabel;

        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;
        Controllers::MotorController& motorController;
        Controllers::MotionController& motionController;

        lv_task_t* taskRefresh;
      };
    }
  }
}
