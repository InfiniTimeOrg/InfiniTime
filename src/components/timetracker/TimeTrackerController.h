#pragma once

#include <FreeRTOS.h>
#include <timers.h>

enum class TimeTrackingMode { Work = 0, Play = 1, Chores = 2, Social = 3, Health = 4, Learn = 5, Iddle = 6 };

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {


    class TimeTrackerController {
    public:
      TimeTrackerController() = default;

      void Reset();
      void Demo();

      void ModeChanged(TimeTrackingMode newMode);
      TickType_t TimeInMode(TimeTrackingMode mode);
      TimeTrackingMode CurrMode();

    private:
      TickType_t startTime = 0;
      TickType_t totals[6] = {0};
      TimeTrackingMode currMode = TimeTrackingMode::Iddle;
    };
  }
}
