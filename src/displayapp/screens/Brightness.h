#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <cstdint>
#include "Screen.h"
#include "components/brightness/BrightnessController.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Brightness : public Screen {
      public:
        Brightness(DisplayApp* app, Controllers::BrightnessController& brightness);
        ~Brightness() override;

        bool OnTouchEvent(TouchEvents event) override;

        void OnValueChanged();

      private:
        Controllers::BrightnessController& brightness;

        lv_obj_t* slider_label;
        lv_obj_t* slider;

        const char* LevelToString(Controllers::BrightnessController::Levels level);
        uint8_t LevelToInt(Controllers::BrightnessController::Levels level);
        void SetValue(uint8_t value);
        void SetValue();
      };
    }
  }
}
