#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

#include <FreeRTOS.h>
#include "portmacro_cmsis.h"

#include "systemtask/SystemTask.h"

namespace Pinetime::Applications::Screens {

  static const char* btnm_map[] = {"Work", "Play", "\n", "Chores", "Social", "\n", "Health", "Learn", ""};

  enum class TrackerStates { Init, Running, Halted };

  struct TimePretty_t {
    int mins;
    int secs;
    int hundredths;
  };

  class TrackTime : public Screen {
  public:
    TrackTime(DisplayApp* app, System::SystemTask& systemTask);
    ~TrackTime() override;
    void Refresh() override;

    void playPauseBtnEventHandler(lv_event_t event);
    void stopLapBtnEventHandler(lv_event_t event);
    void handleModeUpdate(lv_event_t event);
    bool OnButtonPushed() override;

    void Reset();
    void Start();
    void Pause();

  private:
    Pinetime::System::SystemTask& systemTask;
    TrackerStates currentState = TrackerStates::Init;
    TickType_t startTime;
    TickType_t oldTimeElapsed = 0;
    static constexpr int maxLapCount = 20;
    TickType_t laps[maxLapCount + 1];
    static constexpr int displayedLaps = 2;
    int lapsDone = 0;
    lv_obj_t *time, *msecTime, *btnPlayPause, *btnStopLap, *txtPlayPause, *txtStopLap;
    lv_obj_t* lapText;
    lv_obj_t* btnm1;
    lv_obj_t* title;

    lv_task_t* taskRefresh;

    uint16_t oldBtnPressed = 0 ;
  };
}
