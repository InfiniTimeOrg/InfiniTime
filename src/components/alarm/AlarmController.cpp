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
#include <libraries/log/nrf_log.h>

using namespace Pinetime::Controllers;
using namespace std::chrono_literals;

AlarmController::AlarmController(Controllers::DateTime& dateTimeController, Controllers::FS& fs)
  : dateTimeController {dateTimeController}, fs {fs} {
}

namespace {
  void SetOffAlarm(TimerHandle_t xTimer) {
    auto controller = static_cast<Pinetime::Controllers::AlarmController*>(pvTimerGetTimerID(xTimer));
    controller->SetOffAlarmNow();
  }
}

void AlarmController::Init(System::SystemTask* systemTask) {
  this->systemTask = systemTask;
  alarmTimer = xTimerCreate("Alarm", 1, pdFALSE, this, SetOffAlarm);
  LoadAlarmFromFile();
  if (alarm.isEnabled) {
    NRF_LOG_INFO("[AlarmController] Loaded alarm was enabled, scheduling");
    ScheduleAlarm();
  }
}

void AlarmController::SetAlarmTime(uint8_t alarmHr, uint8_t alarmMin) {
  if (alarm.hours == alarmHr && alarm.minutes == alarmMin) {
    return;
  }
  alarm.hours = alarmHr;
  alarm.minutes = alarmMin;
  SaveAlarmToFile();
}

void AlarmController::ScheduleAlarm() {
  // Determine the next time the alarm needs to go off and set the timer
  xTimerStop(alarmTimer, 0);

  auto now = dateTimeController.CurrentDateTime();
  alarmTime = now;
  time_t ttAlarmTime = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(alarmTime));
  tm* tmAlarmTime = std::localtime(&ttAlarmTime);

  // If the time being set has already passed today,the alarm should be set for tomorrow
  if (alarm.hours < dateTimeController.Hours() ||
      (alarm.hours == dateTimeController.Hours() && alarm.minutes <= dateTimeController.Minutes())) {
    tmAlarmTime->tm_mday += 1;
    // tm_wday doesn't update automatically
    tmAlarmTime->tm_wday = (tmAlarmTime->tm_wday + 1) % 7;
  }

  tmAlarmTime->tm_hour = alarm.hours;
  tmAlarmTime->tm_min = alarm.minutes;
  tmAlarmTime->tm_sec = 0;

  // if alarm is in weekday-only mode, make sure it shifts to the next weekday
  if (alarm.recurrence == RecurType::Weekdays) {
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

  if (!alarm.isEnabled) {
    alarm.isEnabled = true;
    SaveAlarmToFile();
  }
}

uint32_t AlarmController::SecondsToAlarm() {
  return std::chrono::duration_cast<std::chrono::seconds>(alarmTime - dateTimeController.CurrentDateTime()).count();
}

void AlarmController::DisableAlarm() {
  xTimerStop(alarmTimer, 0);
  isAlerting = false;
  if (alarm.isEnabled) {
    alarm.isEnabled = false;
    SaveAlarmToFile();
  }
}

void AlarmController::SetOffAlarmNow() {
  isAlerting = true;
  systemTask->PushMessage(System::Messages::SetOffAlarm);
}

void AlarmController::StopAlerting() {
  isAlerting = false;
  // Disable alarm unless it is recurring
  if (alarm.recurrence == RecurType::None) {
    alarm.isEnabled = false;
    SaveAlarmToFile();
  } else {
    // set next instance
    ScheduleAlarm();
  }
  systemTask->PushMessage(System::Messages::StopRinging);
}

void AlarmController::SetRecurrence(RecurType recurrence) {
  if (alarm.recurrence != recurrence) {
    alarm.recurrence = recurrence;
    SaveAlarmToFile();
  }
}

void AlarmController::LoadAlarmFromFile() {
  lfs_file_t file;
  AlarmData buffer;

  if (fs.FileOpen(&file, "/alarm.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[AlarmController] Failed to open alarm data file");
    return;
  }

  fs.FileRead(&file, reinterpret_cast<uint8_t*>(&buffer), sizeof buffer);
  fs.FileClose(&file);
  if (buffer.version != alarmFormatVersion) {
    NRF_LOG_WARNING("[AlarmController] Loaded alarm data has version %u instead of %u, discarding", buffer.version, alarmFormatVersion);
    return;
  }

  alarm = buffer;
  NRF_LOG_INFO("[AlarmController] Loaded alarm data from file");
}

void AlarmController::SaveAlarmToFile() {
  lfs_file_t file;
  if (fs.FileOpen(&file, "/alarm.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[AlarmController] Failed to open alarm data file for saving");
    return;
  }

  fs.FileWrite(&file, reinterpret_cast<uint8_t*>(&alarm), sizeof alarm);
  fs.FileClose(&file);
  NRF_LOG_INFO("[AlarmController] Saved alarm data with format version %u to file", alarm.version);
}
