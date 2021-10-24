#pragma once

#include <cstdint>
// #include "portmacro_cmsis.h"
#include "FreeRTOS.h"

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {

    enum class StopWatchStates { Cleared, Running, Paused };

    class StopWatch {
    public:
      StopWatch() = default;

      // void init();
      void start(uint32_t start);
      // void lap(uint32_t lapEnd);
      void pause(uint32_t end);
      void clear();

      uint32_t getStart();
      uint32_t getElapsedPreviously();

      bool isRunning();
      bool isCleared();
      bool isPaused();

    private:
        StopWatchStates currentState = StopWatchStates::Cleared;
      // Start time of current duration
        TickType_t startTime;
      // How much time was elapsed before current duration
        TickType_t timeElapsedPreviously = 0;
    };
  }
}
