#pragma once
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Controllers {
    class AlarmController {
    public:
      AlarmController(Controllers::DateTime& dateTimeController);

      void Init(System::SystemTask* systemTask);
      void SetAlarmTime(uint8_t alarmHr, uint8_t alarmMin);
      void ScheduleAlarm();
      void DisableAlarm();
      void SetOffAlarmNow();
      uint32_t SecondsToAlarm() const;
      void StopAlerting();
      enum class AlarmState { Not_Set, Set, Alerting };
      enum class RecurType { None, Daily, Weekdays };

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

      void SetRecurrence(RecurType recurType) {
        recurrence = recurType;
      }

    private:
      Controllers::DateTime& dateTimeController;
      System::SystemTask* systemTask = nullptr;
      TimerHandle_t alarmTimer;
      uint8_t hours = 7;
      uint8_t minutes = 0;
      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> alarmTime;
      AlarmState state = AlarmState::Not_Set;
      RecurType recurrence = RecurType::None;
    };
  }
}
