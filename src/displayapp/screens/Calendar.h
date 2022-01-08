#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
  }

  namespace Applications {
    namespace Screens {
      class Calendar : public Screen {
      public:
        Calendar(DisplayApp* app,
            Controllers::DateTime& dateTimeController);
        ~Calendar() override;
      private:
        Controllers::DateTime& dateTimeController;
      };
    }
  }
}
