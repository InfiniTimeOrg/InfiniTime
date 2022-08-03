#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include "touchhandler/TouchHandler.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ScreenLockConfirmation : public Screen {
      public:
        ScreenLockConfirmation(DisplayApp* app, Controllers::TouchHandler& touchHandler);
        ~ScreenLockConfirmation() override;

        void OnButtonEvent(lv_obj_t* obj);

      private:
        DisplayApp* app;
        lv_obj_t* label;
        lv_obj_t* btnEnable;
        lv_obj_t* txtEnable;

        Controllers::TouchHandler& touchHandler;
      };
    }
  }
}
