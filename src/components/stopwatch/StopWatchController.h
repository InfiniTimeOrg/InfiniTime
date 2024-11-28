#pragma once

#include <FreeRTOS.h>
#include <optional>
#include <timers.h>
#include "utility/CircularBuffer.h"

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Controllers {

    enum class StopWatchStates { Cleared, Running, Paused };

    struct LapInfo {
      int number = 0;                // Used to label the lap
      TickType_t timeSinceStart = 0; // Excluding pauses
    };

    class StopWatchController {
    public:
      StopWatchController();

      // StopWatch functionality and data
      void Start();
      void Pause();
      void Clear();

      TickType_t GetElapsedTime();

      // Lap functionality

      /// Only the latest histSize laps are stored
      void AddLapToHistory();

      /// Returns maxLapNumber
      int GetMaxLapNumber();

      /// Indexes into lap history, with 0 being the latest lap.
      std::optional<LapInfo> GetLapFromHistory(int index);

      bool IsRunning();
      bool IsCleared();
      bool IsPaused();

    private:
      // Current state of stopwatch
      StopWatchStates currentState = StopWatchStates::Cleared;
      // Start time of current duration
      TickType_t startTime;
      // How much time was elapsed before current duration
      TickType_t timeElapsedPreviously;

      // Maximum number of stored laps
      static constexpr int histSize = 2;
      static constexpr int lapNumberBoundary = 1000;
      // Lap storage
      Utility::CircularBuffer<LapInfo, histSize> history;
      // Highest lap number; less than lapNumberBoundary, may exceed histSize
      int maxLapNumber;
    };
  }
}
