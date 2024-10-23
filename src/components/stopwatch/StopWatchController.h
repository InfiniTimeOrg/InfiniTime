#pragma once

#include <FreeRTOS.h>
#include <optional>
#include <timers.h>

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {

    enum class StopWatchStates { Cleared, Running, Paused };

    struct LapInfo {
      int count = 0;       // Used to label the lap
      TickType_t time = 0; // Delta time from beginning of stopwatch
    };

    constexpr int lapCapacity = 2;

    class StopWatchController {
    public:
      StopWatchController();

      // StopWatch functionality and data
      void Start();
      void Pause();
      void Clear();

      TickType_t GetElapsedTime();

      // Lap functionality

      /// Only the latest laps are stored, the lap count is saved until reset
      void PushLap();

      /// Returns lapCount
      int GetLapCount();

      /// Indexes into lap history, with 0 being the latest lap.
      std::optional<LapInfo> LastLap(int lap = 0);

      bool IsRunning();
      bool IsCleared();
      bool IsPaused();

    // private:
      // Current state of stopwatch
      StopWatchStates currentState = StopWatchStates::Cleared;
      // Start time of current duration
      TickType_t startTime = 0;
      // How much time was elapsed before current duration
      TickType_t timeElapsedPreviously = 0;
      // Stores lap times
      LapInfo laps[lapCapacity];
      // Total lap count; may exceed lapCapacity
      int lapCount = 0;
      // Index for next lap time; must be lower than lapCapacity
      int lapHead = 0;
    };
  }
}
