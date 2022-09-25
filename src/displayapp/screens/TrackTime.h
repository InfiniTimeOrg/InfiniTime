#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

#include <FreeRTOS.h>
#include "portmacro_cmsis.h"

#include "systemtask/SystemTask.h"
#include "components/timetracker/TimeTrackerController.h"

namespace Pinetime::Applications::Screens {

  static const char* btnm_map[] = {"Work", "Play", "\n", "Chores", "Social", "\n", "Health", "Learn", ""};

  struct TimePretty_t {
    int hours;
    int mins;
    int secs;
  };

  //enum TimeTrackingMode { Work = 0, Play = 1, Chores = 2, Social = 3, Health = 4, Learn = 5, Total };

  class TrackTime : public Screen {
  public:
    TrackTime(DisplayApp* app, System::SystemTask& systemTask, Controllers::TimeTrackerController& timeTrackerController);
    ~TrackTime() override;

    void ShowInfo();
    void CloseInfo();

    void handleModeUpdate(lv_obj_t* obj, lv_event_t event);

  private:
    Pinetime::System::SystemTask& systemTask;
    Controllers::TimeTrackerController& timeTrackerController;
    // TickType_t startTime;
    // TimeTrackingMode currMode = TimeTrackingMode::Total;
    // TickType_t totals[TimeTrackingMode::Total];
    lv_obj_t* btnm1;
    lv_obj_t* title;
    lv_obj_t* txtMessage = nullptr;
    lv_obj_t* btnMessage = nullptr;
    lv_obj_t* btnSummary = nullptr;
    lv_obj_t* txtSummary = nullptr;
    lv_obj_t* btnReset = nullptr;
    lv_obj_t* txtReset = nullptr;
    TimeTrackingMode currMode = TimeTrackingMode::Iddle;

    lv_task_t* taskRefresh;
  };
}
