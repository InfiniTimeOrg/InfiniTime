#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include "portmacro_cmsis.h"

namespace Pinetime::Applications::Screens {

  enum TimeTrackingModes {
    Work,
    Play,
    Chores,
    Social,
    Health,
    Learn,
    Total
  };

  struct TimeTrackedPretty {
    int hours;
    int mins;
    int secs;
  };

  class TrackTime : public Screen {

  public:
    TrackTime(DisplayApp* app);
    ~TrackTime() override;

    void OnEvent(lv_obj_t* obj, lv_event_t event);

  private:
    int currentMode = TimeTrackingModes::Learn;
    uint16_t oldBtnPressed = 0;
    TickType_t oldTimes[TimeTrackingModes::Total] = {0};
    TickType_t totals[TimeTrackingModes::Total] = {0};
    lv_obj_t* btnm1;
  };
}