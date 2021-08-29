#pragma once

#include "Screen.h"
#include "components/timer/TimerController.h"
#include "lvgl/lvgl.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Timer : public Screen {
      public:
        enum {secUp, secDown, minUp, minDown};
        Timer(DisplayApp* app, Controllers::TimerController& timerController);

        ~Timer() override;

        void Refresh() override;

        void SetDone();

        void PlayPauseEventHandler(lv_obj_t* obj, lv_event_t event);
        void AdjustButtonEventHandler(lv_obj_t* obj, lv_event_t event);

      private:
        uint8_t seconds = 0;
        uint8_t minutes = 0;
        Controllers::TimerController& timerController;

        DirtyValue<uint32_t> remainingTime;

        void SetButtonsHidden(bool hidden);
        void UpdateTime();

        lv_obj_t* time;
        lv_obj_t* btnPlayPause;
        lv_obj_t* buttons[4];
        lv_task_t* taskRefresh;
      };
    }
  }
}
