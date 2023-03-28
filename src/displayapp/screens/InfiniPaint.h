#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <algorithm> // std::fill
#include "displayapp/screens/Screen.h"
#include "components/motor/MotorController.h"

namespace Pinetime {
  namespace Components {
    class LittleVgl;
  }

  namespace Applications {
    namespace Screens {

      class InfiniPaint : public Screen {
      public:
        InfiniPaint(Pinetime::Components::LittleVgl& lvgl, Controllers::MotorController& motor);

        ~InfiniPaint() override;

        bool OnTouchEvent(TouchEvents event) override;

        bool OnTouchEvent(uint16_t x, uint16_t y) override;

      private:
        Pinetime::Components::LittleVgl& lvgl;
        Controllers::MotorController& motor;
        static constexpr uint16_t width = 10;
        static constexpr uint16_t height = 10;
        static constexpr uint16_t bufferSize = width * height;
        lv_color_t b[bufferSize];
        lv_color_t selectColor = LV_COLOR_WHITE;
        uint8_t color = 2;
      };
    }
  }
}
