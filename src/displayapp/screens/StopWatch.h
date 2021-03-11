#pragma once

#include "Screen.h"
#include "components/datetime/DateTimeController.h"
#include "../LittleVgl.h"

#include "FreeRTOS.h"
#include "portmacro_cmsis.h"

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
    const Pinetime::Controllers::DateTime& dateTime;
    bool running;
    States currentState;
    Events currentEvent;
    TickType_t startTime;
    TickType_t timeElapsed;
    lv_obj_t *time, *msecTime;
  };
}