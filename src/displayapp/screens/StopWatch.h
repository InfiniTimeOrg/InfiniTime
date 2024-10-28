#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

#include "components/stopwatch/StopWatchController.h"
#include "systemtask/SystemTask.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"

namespace Pinetime::Applications {
  namespace Screens {

    struct TimeSeparated {
      int hours;
      int mins;
      int secs;
      int hundredths;
    };

    class StopWatch : public Screen {
    public:
      explicit StopWatch(System::SystemTask& systemTask, Controllers::StopWatchController& stopWatchController);
      ~StopWatch() override;
      void Refresh() override;

      void PlayPauseBtnEventHandler();
      void StopLapBtnEventHandler();
      bool OnButtonPushed() override;

    private:
      void DisplayPaused();
      void DisplayStarted();
      void DisplayCleared();

      void RenderTime();
      void RenderPause();
      void RenderLaps();

      Pinetime::System::WakeLock wakeLock;
      Controllers::StopWatchController& stopWatchController;
      TickType_t blinkTime = 0;
      static constexpr int displayedLaps = 2;
      lv_obj_t *time, *msecTime, *btnPlayPause, *btnStopLap, *txtPlayPause, *txtStopLap;
      lv_obj_t* lapText;
      bool isHoursLabelUpdated = false;

      lv_task_t* taskRefresh;
    };
  }

  template <>
  struct AppTraits<Apps::StopWatch> {
    static constexpr Apps app = Apps::StopWatch;
    static constexpr const char* icon = Screens::Symbols::stopWatch;

    static Screens::Screen* Create(AppControllers& controllers) {
      return new Screens::StopWatch(*controllers.systemTask, controllers.stopWatchController);
    }
    static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
      return true;
    }
  };
}
