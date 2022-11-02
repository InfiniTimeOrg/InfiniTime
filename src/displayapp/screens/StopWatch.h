#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

#include "components/stopwatch/StopWatchController.h"
#include "systemtask/SystemTask.h"

namespace Pinetime::Applications::Screens {

  struct TimeSeparated {
    int mins;
    int secs;
    int hundredths;
  };

  class StopWatch : public Screen {
  public:
    StopWatch(DisplayApp* app, System::SystemTask& systemTask,
              Controllers::StopWatchController& stopWatchController);
    ~StopWatch() override;
    void Refresh() override;

    void PlayPauseBtnEventHandler(lv_event_t event);
    void StopLapBtnEventHandler(lv_event_t event);
    bool OnButtonPushed() override;

    void DisplayReset();
    void DisplayStart();
    void DisplayPause();

  private:
    Pinetime::System::SystemTask& systemTask;
    Controllers::StopWatchController& stopWatchController;
    static constexpr int displayedLaps = 2;
    TickType_t timeElapsed;
    lv_obj_t *time, *msecTime, *btnPlayPause, *btnStopLap, *txtPlayPause, *txtStopLap;
    lv_obj_t* lapText;

    lv_task_t* taskRefresh;

    void UpdateTime();
    void UpdateLaps();
  };
}
