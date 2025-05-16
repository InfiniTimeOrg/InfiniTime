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
      uint16_t number = 0;           // Used to label the lap
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
      uint16_t GetMaxLapNumber();

      /// Indexes into lap history, with 0 being the latest lap.
      std::optional<LapInfo> GetLapFromHistory(uint8_t index);

      bool IsRunning();
      bool IsCleared();
      bool IsPaused();

    private:
      // Time at which stopwatch wraps around to zero (1000 hours)
      static constexpr TickType_t elapsedTimeBoundary = static_cast<TickType_t>(configTICK_RATE_HZ) * 60 * 60 * 1000;
      // Current state of stopwatch
      StopWatchStates currentState = StopWatchStates::Cleared;
      // Start time of current duration
      TickType_t startTime;
      // How much time was elapsed before current duration
      TickType_t timeElapsedPreviously;

      // Maximum number of stored laps
      static constexpr uint8_t histSize = 4;
      // Value at which lap numbers wrap around to zero
      static constexpr uint16_t lapNumberBoundary = 1000;
      // Lap storage
      Utility::CircularBuffer<LapInfo, histSize> history;
      // Highest lap number; less than lapNumberBoundary, may exceed histSize
      uint16_t maxLapNumber;
    };
  }
}
