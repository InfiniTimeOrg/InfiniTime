#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <FreeRTOS.h>
#include <timers.h>
#include "components/timer/Timer.h"
#include "components/settings/Settings.h"
#include "components/pomodoro/SessionStatistics.h"

namespace Pinetime {
  namespace Controllers {
    class DateTime;
    
    class PomodoroController {
    public:
      enum class SessionType : uint8_t {
        Work,
        ShortBreak,
        LongBreak
      };
      
      enum class SessionState : uint8_t {
        Ready,
        Active,
        Paused,
        Completed
      };
      
      using TimerCallback = std::function<void()>;
      
      PomodoroController(Settings& settingsController, DateTime& dateTimeController);
      
      // Session control methods
      void StartWorkSession();
      void StartBreak();
      void PauseSession();
      void ResumeSession();
      void CancelSession();
      
      // State query methods
      SessionState GetCurrentState() const;
      SessionType GetCurrentSessionType() const;
      std::chrono::milliseconds GetTimeRemaining();
      
      // Session tracking methods
      uint8_t GetCompletedWorkSessions() const;
      uint8_t GetCurrentCyclePosition() const;
      bool IsLongBreakDue() const;
      void ResetSessionCounter();
      
      // Statistics methods
      uint8_t GetDailyCompletedSessions() const;
      std::chrono::minutes GetDailyWorkTime() const;
      void CheckAndResetDailyStatistics();
      
      // Configuration methods
      void SetSessionsBeforeLongBreak(uint8_t sessions);
      
      // Callback registration
      void SetSessionCompleteCallback(TimerCallback callback);
      
      // Timer callback handler (called by Timer component)
      void OnTimerCallback();
      
    private:
      Timer timer;
      Settings& settingsController;
      SessionStatistics sessionStatistics;
      
      SessionState currentState;
      SessionType currentSessionType;
      std::chrono::milliseconds pausedTimeRemaining;
      
      // Session tracking
      uint8_t completedWorkSessions;
      
      // Callback for session completion notifications
      TimerCallback sessionCompleteCallback;
      
      // Internal state management
      void TransitionToReady();
      void TransitionToActive();
      void TransitionToPaused();
      void TransitionToCompleted();
      
      // Helper methods
      std::chrono::milliseconds GetSessionDuration(SessionType type) const;
      SessionType DetermineBreakType() const;
      
      // Static timer callback for FreeRTOS timer
      static void TimerCallbackStatic(TimerHandle_t xTimer);
    };
  }
}