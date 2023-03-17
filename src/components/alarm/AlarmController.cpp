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
#include "components/alarm/AlarmController.h"
#include "systemtask/SystemTask.h"
#include "task.h"
#include <chrono>

using namespace Pinetime::Controllers;
using namespace std::chrono_literals;

AlarmController::AlarmController(Controllers::DateTime& dateTimeController) : dateTimeController {dateTimeController} {
}

namespace {
  void SetOffAlarm(TimerHandle_t xTimer) {
    auto* controller = static_cast<Pinetime::Controllers::AlarmController*>(pvTimerGetTimerID(xTimer));
    controller->SetOffAlarmNow();
  }
}

void AlarmController::Init(System::SystemTask* systemTask) {
  this->systemTask = systemTask;
  alarmTimer = xTimerCreate("Alarm", 1, pdFALSE, this, SetOffAlarm);
}

void AlarmController::SetAlarmTime(uint8_t alarmHr, uint8_t alarmMin) {
  hours = alarmHr;
  minutes = alarmMin;
}

void AlarmController::ScheduleAlarm() {
  // Determine the next time the alarm needs to go off and set the timer
  xTimerStop(alarmTimer, 0);

  auto now = dateTimeController.CurrentDateTime();
  alarmTime = now;
  time_t ttAlarmTime = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(alarmTime));
  tm* tmAlarmTime = std::localtime(&ttAlarmTime);

  // If the time being set has already passed today,the alarm should be set for tomorrow
  if (hours < dateTimeController.Hours() || (hours == dateTimeController.Hours() && minutes <= dateTimeController.Minutes())) {
    tmAlarmTime->tm_mday += 1;
    // tm_wday doesn't update automatically
    tmAlarmTime->tm_wday = (tmAlarmTime->tm_wday + 1) % 7;
  }

  tmAlarmTime->tm_hour = hours;
  tmAlarmTime->tm_min = minutes;
  tmAlarmTime->tm_sec = 0;

  // if alarm is in weekday-only mode, make sure it shifts to the next weekday
  if (recurrence == RecurType::Weekdays) {
    if (tmAlarmTime->tm_wday == 0) { // Sunday, shift 1 day
      tmAlarmTime->tm_mday += 1;
    } else if (tmAlarmTime->tm_wday == 6) { // Saturday, shift 2 days
      tmAlarmTime->tm_mday += 2;
    }
  }
  tmAlarmTime->tm_isdst = -1; // use system timezone setting to determine DST

  // now can convert back to a time_point
  alarmTime = std::chrono::system_clock::from_time_t(std::mktime(tmAlarmTime));
  auto secondsToAlarm = std::chrono::duration_cast<std::chrono::seconds>(alarmTime - now).count();
  xTimerChangePeriod(alarmTimer, secondsToAlarm * configTICK_RATE_HZ, 0);
  xTimerStart(alarmTimer, 0);

  state = AlarmState::Set;
}

uint32_t AlarmController::SecondsToAlarm() const {
  return std::chrono::duration_cast<std::chrono::seconds>(alarmTime - dateTimeController.CurrentDateTime()).count();
}

void AlarmController::DisableAlarm() {
  xTimerStop(alarmTimer, 0);
  state = AlarmState::Not_Set;
}

void AlarmController::SetOffAlarmNow() {
  state = AlarmState::Alerting;
  systemTask->PushMessage(System::Messages::SetOffAlarm);
}

void AlarmController::StopAlerting() {
  // Alarm state is off unless this is a recurring alarm
  if (recurrence == RecurType::None) {
    state = AlarmState::Not_Set;
  } else {
    // set next instance
    ScheduleAlarm();
  }
}
