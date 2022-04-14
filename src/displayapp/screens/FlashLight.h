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

      private:
        void SetColors();
        void SetBrightness();

        Pinetime::System::SystemTask& systemTask;
        Controllers::BrightnessController& brightnessController;

        lv_obj_t* flashLight;
        lv_obj_t* backgroundLabel;
        bool isOn = false;
        bool highBrightness = true;
        bool firstTapDone = false;
      };
    }
  }
}
