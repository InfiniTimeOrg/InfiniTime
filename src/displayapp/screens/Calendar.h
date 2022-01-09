#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include "components/datetime/DateTimeController.h"
#include "components/battery/BatteryController.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
  }

  namespace Applications {
    namespace Screens {
      class Calendar : public Screen {
      public:
        Calendar(DisplayApp* app,
            Pinetime::Controllers::Battery& batteryController,
            Controllers::DateTime& dateTimeController);
        ~Calendar() override;
      private:
        Pinetime::Controllers::Battery& batteryController;
        Controllers::DateTime& dateTimeController;
        lv_obj_t* batteryIcon;
        lv_obj_t* label_time;
      };
    }
  }
}
