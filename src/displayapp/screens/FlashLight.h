#pragma once

#include "displayapp/screens/Screen.h"
#include "components/brightness/BrightnessController.h"
#include "systemtask/SystemTask.h"
#include <cstdint>
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Controllers {
    class InfinitimeService;
  }

  namespace Applications {
    namespace Screens {

      class FlashLight : public Screen {
      public:
        FlashLight(DisplayApp* app,
                   System::SystemTask& systemTask,
                   Controllers::BrightnessController& brightness,
                   Pinetime::Controllers::InfinitimeService& infinitime);
        ~FlashLight() override;

        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;
        void Toggle();

      private:
        void SetIndicators();
        void SetColors();

        Pinetime::System::SystemTask& systemTask;
        Controllers::BrightnessController& brightnessController;
        Pinetime::Controllers::InfinitimeService& infinitimeService;

        Controllers::BrightnessController::Levels brightnessLevel = Controllers::BrightnessController::Levels::High;

        lv_obj_t* flashLight;
        lv_obj_t* backgroundAction;
        lv_obj_t* indicators[3];
        bool isOn = false;
      };
    }
  }
}
