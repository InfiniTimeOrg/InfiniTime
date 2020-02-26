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

      class Meter : public Screen{
        public:
          Meter(DisplayApp* app);
          ~Meter() override;

          bool Refresh() override;
          bool OnButtonPushed() override;

        private:
          lv_style_t style_lmeter;
          lv_obj_t * lmeter;

          uint32_t value=0;
          bool running = true;

      };
    }
  }
}
