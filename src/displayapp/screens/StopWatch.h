#pragma once

#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "displayapp/LittleVgl.h"
#include "components/stopwatch/StopWatchController.h"
#include "components/battery/BatteryController.h"
#include "displayapp/LittleVgl.h"

#include "FreeRTOS.h"
#include "portmacro_cmsis.h"

#include <array>
#include "systemtask/SystemTask.h"

namespace Pinetime::Applications::Screens {

  struct TimeSeparated_t {
    int mins;
    int secs;
    int hundredths;
  };

  class StopWatch : public Screen {
  public:
    StopWatch(DisplayApp* app,
              System::SystemTask& systemTask,
              Controllers::DateTime& dateTimeController,
              Controllers::StopWatch& stopWatchController,
              Controllers::Battery& batteryController);
    ~StopWatch() override;
    void Refresh() override;

    void playPauseBtnEventHandler(lv_event_t event);
    void stopLapBtnEventHandler(lv_event_t event);

    void displayCleared();
    void displayRunning();
    void displayPaused();
    void updateLaps();

  private:
    Pinetime::System::SystemTask& systemTask;
    Controllers::DateTime& dateTimeController;
    Controllers::StopWatch& stopWatchController;
    Controllers::Battery& batteryController;

    TickType_t timeElapsed;
    TimeSeparated_t currentTimeSeparated; // Holds Mins, Secs, millisecs

    lv_obj_t *dateTime, *time, *msecTime, *btnPlayPause, *btnStopLap, *txtPlayPause, *txtStopLap, *batteryIcon;
    lv_obj_t *lapOneText, *lapTwoText;

    lv_task_t* taskRefresh;
  };
}
