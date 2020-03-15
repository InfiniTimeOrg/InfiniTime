#pragma once

#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <Components/Brightness/BrightnessController.h>
#include "Screen.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Brightness : public Screen {
        public:
          Brightness(DisplayApp* app, Controllers::BrightnessController& brightness);
          ~Brightness() override;
          bool Refresh() override;
          bool OnButtonPushed() override;

          void OnValueChanged();
        private:
          bool running = true;
          Controllers::BrightnessController& brightness;

          lv_obj_t * slider_label;
          lv_obj_t * slider;

          const char* LevelToString(Controllers::BrightnessController::Levels level);
          uint8_t LevelToInt(Controllers::BrightnessController::Levels level);
          void SetValue(uint8_t value);
      };
    }
  }
}