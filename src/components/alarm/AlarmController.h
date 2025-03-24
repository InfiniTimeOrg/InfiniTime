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
      AlarmController(Controllers::DateTime& dateTimeController, Controllers::FS& fs);

      void Init(System::SystemTask* systemTask);
      void SaveAlarm();
      void SetAlarmTime(uint8_t alarmHr, uint8_t alarmMin);
      void ScheduleAlarm();
      void DisableAlarm();
      void SetOffAlarmNow();
      uint32_t SecondsToAlarm() const;
      void StopAlerting();
      enum class RecurType { None, Daily, Weekdays };

      uint8_t Hours() const {
        return alarm.hours;
      }

      uint8_t Minutes() const {
        return alarm.minutes;
      }

      bool IsAlerting() const {
        return isAlerting;
      }

      bool IsEnabled() const {
        return alarm.isEnabled;
      }

      RecurType Recurrence() const {
        return alarm.recurrence;
      }

      void SetRecurrence(RecurType recurrence);

    private:
      // Versions 255 is reserved for now, so the version field can be made
      // bigger, should it ever be needed.
      static constexpr uint8_t alarmFormatVersion = 1;

      struct AlarmSettings {
        uint8_t version = alarmFormatVersion;
        uint8_t hours = 7;
        uint8_t minutes = 0;
        RecurType recurrence = RecurType::None;
        bool isEnabled = false;
      };

      bool isAlerting = false;
      bool alarmChanged = false;

      Controllers::DateTime& dateTimeController;
      Controllers::FS& fs;
      System::SystemTask* systemTask = nullptr;
      TimerHandle_t alarmTimer;
      AlarmSettings alarm;
      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> alarmTime;

      void LoadSettingsFromFile();
      void SaveSettingsToFile() const;
    };
  }
}
