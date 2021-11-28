#pragma once

#include "displayapp/screens/Screen.h"
#include "components/brightness/BrightnessController.h"
#include "systemtask/SystemTask.h"
#include <cstdint>
#include <lvgl/lvgl.h>

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class FlashLight : public Screen {
      public:
        FlashLight(DisplayApp* app, System::SystemTask& systemTask, Controllers::BrightnessController& brightness);
        ~FlashLight() override;

        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;
        void OnClickEvent(lv_obj_t* obj, lv_event_t event);

      private:
        void SetIndicators();
        void SetColors();

        Pinetime::System::SystemTask& systemTask;
        Controllers::BrightnessController& brightnessController;

        Controllers::BrightnessController::Levels brightnessLevel;

        lv_obj_t* flashLight;
        lv_obj_t* backgroundAction;
        lv_obj_t* indicators[3];
        bool isOn = false;
      };
    }
  }
}
