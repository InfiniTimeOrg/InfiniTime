#pragma once

#include <FreeRTOS.h>
#include <timers.h>

#include <chrono>

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    class TimerController {
    public:
      TimerController() = default;

      void Init(System::SystemTask* systemTask);

      void StartTimer(std::chrono::hours hours, std::chrono::minutes minutes);

      void StartTimer(std::chrono::milliseconds duration);

      void StopTimer();

      std::chrono::milliseconds GetTimeRemaining();

      bool IsRunning();

      void OnTimerEnd();

      enum class TimerState { Dormant, Running, Alerting, Snoozed };

      void SnoozeAlert();

      void StopAlerting();

      std::chrono::hours Hours() const{
        return hours;
      }

      std::chrono::minutes Minutes() const{
        return minutes;
      }

      TimerState State() const{
        return state;
      }

    private:
      System::SystemTask* systemTask = nullptr;
      TimerHandle_t timer;
      bool useAlert = true;
      std::chrono::hours hours = std::chrono::hours(0);
      std::chrono::minutes minutes = std::chrono::minutes(0);
      std::chrono::milliseconds snoozeTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::minutes(10));
      TimerState state = TimerState::Dormant;
    };
  }
}
