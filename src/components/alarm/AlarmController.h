/*  Copyright (C) 2021 mruss77, Florian

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <cstdint>
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

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
