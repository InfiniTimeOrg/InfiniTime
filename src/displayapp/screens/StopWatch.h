#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

#include "components/stopwatch/StopWatchController.h"
#include "systemtask/SystemTask.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"
#include "utility/DirtyValue.h"

namespace Pinetime::Applications {
  namespace Screens {

    struct TimeSeparated {
      uint16_t hours;
      uint8_t mins;
      uint8_t secs;
      uint8_t hundredths;
      uint32_t epochSecs;
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
      void OnPause();

      void DisplayPaused();
      void DisplayStarted();
      void DisplayCleared();

      void RenderTime();
      void RenderPause();
      void RenderLaps();

      void SetHoursVisible(bool visible);

      Pinetime::System::WakeLock wakeLock;
      Controllers::StopWatchController& stopWatchController;
      TickType_t lastBlinkTime = 0;
      uint8_t displayedLaps = 3;
      lv_obj_t *time, *msecTime, *btnPlayPause, *btnStopLap, *txtPlayPause, *txtStopLap;
      lv_obj_t* lapText;
      Utility::DirtyValue<uint32_t> renderedSeconds;
      bool hoursVisible = false;

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
