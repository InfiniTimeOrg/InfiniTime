#pragma once

#include <cstdint>
#include "app_timer.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    class AlarmController {
    public:
      AlarmController(Controllers::DateTime& dateTimeController);

      void Init();
      void SetAlarm(uint8_t alarmHr, uint8_t alarmMin);
      void DisableAlarm();
      void SetOffAlarmNow();
      uint32_t SecondsToAlarm();
      void StopAlerting();
      void Register(System::SystemTask* systemTask);
      enum class AlarmState { Not_Set, Set, Alerting };
      enum class RecurType { None, Daily, Weekdays };
      void ToggleRecurrence();
      uint8_t Hours() const {
        return hours;
      }
      uint8_t Minutes() const {
        return minutes;
      }
      AlarmState State() const {
        return state;
      }
      RecurType Recurrence() const {
        return recurrence;
      }

    private:
      Controllers::DateTime& dateTimeController;
      System::SystemTask* systemTask = nullptr;
      uint8_t hours;
      uint8_t minutes;
      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> alarmTime;
      AlarmState state = AlarmState::Not_Set; 
      RecurType recurrence = RecurType::None;
      void scheduleAlarm();
    };
  }
}