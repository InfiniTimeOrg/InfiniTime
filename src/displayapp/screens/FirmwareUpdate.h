#pragma once

#include "Screen.h"
#include <lvgl/src/lv_core/lv_obj.h>

namespace Pinetime {
  namespace Controllers {
    class Ble;
  }
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
