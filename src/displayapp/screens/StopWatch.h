#pragma once

#include "displayapp/screens/Screen.h"
#include "lvgl/lvgl.h"
#include "systemtask/SystemTask.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      enum class States { Init, Running, Halted };

      struct TimeSeparated_t {
        uint32_t mins;
        uint8_t secs;
        uint8_t hundredths;
      };

      class StopWatch : public Screen {
      public:
        StopWatch(DisplayApp* app, System::SystemTask& systemTask);
        ~StopWatch() override;
        void Refresh() override;

        void PlayPauseBtnEventHandler(lv_event_t event);
        void StopLapBtnEventHandler(lv_event_t event);
        bool OnButtonPushed() override;
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

      private:
        void DisplayLaps(uint8_t startLap);
        void SetTime(TickType_t timeToSet);

        void Reset();
        void Start();
        void Pause();

        Pinetime::System::SystemTask& systemTask;
        States currentState = States::Init;
        TickType_t startTime = 0;
        TickType_t oldTimeElapsed = 0;

        static constexpr uint8_t maxLapCount = 20;
        TickType_t lapTimes[maxLapCount + 1] = {0};

        uint8_t savedLapsCount = 0;
        uint8_t displayedLapsStart = 0;

        lv_obj_t *time, *msecTime, *btnPlayPause, *btnStopLap;
        lv_obj_t* lapText;
      };
    }
  }
}
