#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/src/lv_core/lv_obj.h>
#include "FreeRTOS.h"

namespace Pinetime {
  namespace Controllers {
    class Ble;
  }

  namespace Applications {
    namespace Screens {

      class FirmwareUpdate : public Screen {
      public:
        FirmwareUpdate(const Pinetime::Controllers::Ble& bleController);
        ~FirmwareUpdate() override;

        void Refresh() override;

      private:
        enum class States { Idle, Running, Validated, Error };
        const Pinetime::Controllers::Ble& bleController;
        lv_obj_t* bar1;
        lv_obj_t* percentLabel;
        lv_obj_t* titleLabel;

        States state = States::Idle;

        void DisplayProgression() const;

        bool OnButtonPushed() override;

        void UpdateValidated();

        void UpdateError();

        lv_task_t* taskRefresh;
        TickType_t startTime;
      };
    }
  }
}
