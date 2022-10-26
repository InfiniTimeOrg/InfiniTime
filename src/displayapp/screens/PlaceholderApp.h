#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class PlaceholderApp : public Screen {
      public:
        PlaceholderApp(DisplayApp* app,
          Pinetime::Controllers::MotorController& motorController);
        ~PlaceholderApp() override;
      };
    }
  }
}
