#pragma once

#include <cstdint>
#include <chrono>
#include "Screen.h"
#include <bits/unique_ptr.h>
#include <libs/lvgl/src/lv_core/lv_style.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>

namespace Pinetime {
  namespace Controllers {
    class HeartRateController;
  }
  namespace Applications {
    namespace Screens {

      class HeartRate : public Screen{
      public:
        HeartRate(DisplayApp* app, Controllers::HeartRateController& HeartRateController);
        ~HeartRate() override;

        bool Refresh() override;
        bool OnButtonPushed() override;
        void OnStartStopEvent(lv_event_t event);

      private:
        Controllers::HeartRateController& heartRateController;
        void UpdateStartStopButton(bool isRunning);
        lv_obj_t* label_hr;
        lv_obj_t* label_bpm;
        lv_obj_t* label_status;
        lv_style_t labelBigStyle;
        lv_style_t* labelStyle;
        lv_obj_t* btn_startStop;
        lv_obj_t* label_startStop;

        bool running = true;

      };
    }
  }
}
