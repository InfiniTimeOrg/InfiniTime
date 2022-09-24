#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime::Applications::Screens {

  enum TrackTimeState {
    Work,
    Chores,
    Social,
    Learn,
    Health,
    Play,
    Iddle,
  }

  class TrackTime : public Screen {

  public:
    TrackTime(DisplayApp* app);
    ~TrackTime() override;


  private:
    States currState = TrackTimeState::Iddle;
    lv_obj_t* buttons[TrackTimeState::Iddle] = {}
    lv_obj_t* textCurrState;
  };
}