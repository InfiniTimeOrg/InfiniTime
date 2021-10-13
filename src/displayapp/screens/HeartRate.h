#pragma once

#include <cstdint>
#include <chrono>
#include "Screen.h"
#include <components/heartrate/HeartRateController.h>
#include <bits/unique_ptr.h>
#include "systemtask/SystemTask.h"
#include <libs/lvgl/src/misc/lv_style.h>
#include <libs/lvgl/src/core/lv_obj.h>

namespace Pinetime {
  namespace Controllers {
    class HeartRateController;
  }
  namespace Applications {
    namespace Screens {

      class HeartRate : public Screen {
      public:
        HeartRate(DisplayApp* app, Controllers::HeartRateController& HeartRateController, System::SystemTask& systemTask);
        ~HeartRate() override;

        void Refresh() override;

        void OnStartStopEvent(lv_event_t* event);

      private:
        Controllers::HeartRateController& heartRateController;
        Pinetime::System::SystemTask& systemTask;
        void UpdateStartStopButton(bool isRunning);
        Controllers::HeartRateController::States prevState = Controllers::HeartRateController::States::Stopped;
        uint8_t prevHeartRate = 0;
        
        lv_obj_t* label_hr;
        lv_obj_t* label_bpm;
        lv_obj_t* label_status;
        lv_obj_t* btn_startStop;
        lv_obj_t* label_startStop;

        lv_timer_t* taskRefresh;
      };
    }
  }
}
