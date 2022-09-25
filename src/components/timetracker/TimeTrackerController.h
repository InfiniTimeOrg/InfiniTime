#pragma once

#include <FreeRTOS.h>
#include <timers.h>

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {

    class TimeTrackerController {
    public:
      TimeTrackerController() = default;

      void Init();
      void Reset();
      void Demo();

      void StartTimer(uint32_t duration);

      void StopTimer();

      uint32_t GetTimeRemaining();

      bool IsRunning();

      void OnTimerEnd();

    private:
      TickType_t startTime;
      TickType_t totals[6] = {};
      uint16_t currMode = 6;
    };
  }
}
