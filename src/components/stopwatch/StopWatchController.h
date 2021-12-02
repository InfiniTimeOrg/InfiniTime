#pragma once

#include <cstdint>
#include "FreeRTOS.h"

#define LAP_CAPACITY 2

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {

    enum class StopWatchStates { Cleared, Running, Paused };

    struct LapInfo_t {
      int count = 0;       // Used to label the lap
      TickType_t time = 0; // delta time from beginning of stopwatch
    };

    class StopWatch {
    public:
      StopWatch();

      // StopWatch functionality and data
      void start(TickType_t start);
      void pause(TickType_t end);
      void clear();

      TickType_t getStart();
      TickType_t getElapsedPreviously();

      // Lap functionality

      /// Only the latest laps are stored, the lap count is saved until reset
      void pushLap(TickType_t lapEnd);

      /// Returns actual count of stored laps
      int getLapNum();

      /// Returns lapCount
      int getLapCount();

      /// Indexes into lap history, with 0 being the latest lap.
      /// If the lap is unavailable, count and time will be 0. If there is a
      /// real value, count should be above 0
      LapInfo_t* lastLap(int lap = 0);

      bool isRunning();
      bool isCleared();
      bool isPaused();

    private:
      // Current state of stopwatch
      StopWatchStates currentState = StopWatchStates::Cleared;
      // Start time of current duration
      TickType_t startTime = 0;
      // How much time was elapsed before current duration
      TickType_t timeElapsedPreviously = 0;
      // Stores lap times
      LapInfo_t laps[LAP_CAPACITY];
      LapInfo_t emptyLapInfo = {.count = 0, .time = 0};
      int lapCount = 0;
      int lapHead = 0;
    };
  }
}
