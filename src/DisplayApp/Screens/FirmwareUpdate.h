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

      class FirmwareUpdate : public Screen{
        public:
          FirmwareUpdate(DisplayApp* app, Pinetime::Controllers::Ble& bleController);
          ~FirmwareUpdate() override;

          bool Refresh() override;
          bool OnButtonPushed() override;

        private:
          enum class States { Idle, Running, Validated, Error };
          Pinetime::Controllers::Ble& bleController;
          lv_obj_t* bar1;
          lv_obj_t* percentLabel;
          lv_obj_t* titleLabel;
          lv_obj_t* labelBtn;
          lv_obj_t* button;
          mutable char percentStr[10];
          bool running = true;
          States state;

          bool DisplayProgression() const;

          void UpdateValidated();

          void UpdateError();
      };
    }
  }
}
