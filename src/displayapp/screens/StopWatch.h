#pragma once

#include "Screen.h"
#include "components/datetime/DateTimeController.h"
#include "../LittleVgl.h"

#include <chrono>

namespace Pinetime::Applications::Screens {

  enum class States { INIT, RUNNING, HALTED };

  enum class Events { PLAY, PAUSE, STOP };

  class StopWatch : public Screen {
  public:
    StopWatch(DisplayApp* app, const Pinetime::Controllers::DateTime& dateTime);
    ~StopWatch() override;
    bool Refresh() override;
    bool OnButtonPushed() override;
    bool OnTouchEvent(uint16_t x, uint16_t y) override;

  private:
    using timeUnit = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
    const Pinetime::Controllers::DateTime& dateTime;
    bool running;
    States currentState;
    Events currentEvent;
    timeUnit startTime;
    int64_t timeElapsed;
    lv_obj_t *time, *msecTime;
  };
}