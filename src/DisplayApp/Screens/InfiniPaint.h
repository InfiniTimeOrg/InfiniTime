#pragma once

#include <cstdint>
#include "Screen.h"
#include <bits/unique_ptr.h>
#include <libs/lvgl/src/lv_core/lv_style.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <drivers/St7789.h>
#include <DisplayApp/LittleVgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class InfiniPaint : public Screen{
        public:
          InfiniPaint(DisplayApp* app, Pinetime::Components::LittleVgl& lvgl);
          ~InfiniPaint() override;

          bool Refresh() override;
          bool OnButtonPushed() override;
          bool OnTouchEvent(TouchEvents event) override;
          bool OnTouchEvent(uint16_t x, uint16_t y) override;

        private:
          Pinetime::Components::LittleVgl& lvgl;
          static constexpr uint16_t width = 10;
          static constexpr uint16_t height = 10;
          static constexpr uint16_t bufferSize = width*height;
          lv_color_t b[bufferSize];
          bool running = true;
      };
    }
  }
}
