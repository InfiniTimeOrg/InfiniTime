#pragma once

#include <cstdint>
#include <chrono>
#include <Components/Gfx/Gfx.h>
#include "Screen.h"
#include <bits/unique_ptr.h>
#include <libs/lvgl/src/lv_core/lv_style.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <Components/Battery/BatteryController.h>
#include <Components/Ble/BleController.h>
#include "../Fonts/lcdfont14.h"
#include "../Fonts/lcdfont70.h"
#include "../../Version.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Gauge : public Screen{
        public:
          Gauge(DisplayApp* app);
          ~Gauge() override;

          bool Refresh() override;
          bool OnButtonPushed() override;

        private:
          lv_style_t style;
          lv_color_t needle_colors[3];
          lv_obj_t * gauge1;

          uint32_t value=30;
          bool running = true;

      };
    }
  }
}
