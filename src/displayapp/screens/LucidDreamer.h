#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include "components/motor/MotorController.h"


namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class LucidDreamer : public Screen {
      public:
        LucidDreamer(DisplayApp* app, Controllers::MotorController& motor);
        ~LucidDreamer() override;
        void OnEvent(lv_obj_t* obj, lv_event_t event);

      private:
        Controllers::MotorController& motor;
        lv_obj_t* play1;
        lv_obj_t* lblplay1;
        lv_obj_t* play2;
        lv_obj_t* lblplay2;
        lv_obj_t* play3;
        lv_obj_t* lblplay3;



      };
    }
  }
}
