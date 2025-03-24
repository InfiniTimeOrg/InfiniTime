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
    auto* controller = static_cast<Pinetime::Controllers::AlarmController*>(pvTimerGetTimerID(xTimer));
    controller->SetOffAlarmNow();
  }
}

void AlarmController::Init(System::SystemTask* systemTask) {
  this->systemTask = systemTask;
  alarmTimer = xTimerCreate("Alarm", 1, pdFALSE, this, SetOffAlarm);
  LoadSettingsFromFile();
  if (alarm.isEnabled) {
    NRF_LOG_INFO("[AlarmController] Loaded alarm was enabled, scheduling");
    ScheduleAlarm();
  }
}

void AlarmController::SaveAlarm() {
  // verify if it is necessary to save
  if (alarmChanged) {
    SaveSettingsToFile();
  }
  alarmChanged = false;
}

void AlarmController::SetAlarmTime(uint8_t alarmHr, uint8_t alarmMin) {
  if (alarm.hours == alarmHr && alarm.minutes == alarmMin) {
    return;
  }
  alarm.hours = alarmHr;
  alarm.minutes = alarmMin;
  alarmChanged = true;
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
    alarmChanged = true;
  }
}

uint32_t AlarmController::SecondsToAlarm() const {
  return std::chrono::duration_cast<std::chrono::seconds>(alarmTime - dateTimeController.CurrentDateTime()).count();
}

void AlarmController::DisableAlarm() {
  xTimerStop(alarmTimer, 0);
  if (alarm.isEnabled) {
    alarm.isEnabled = false;
    alarmChanged = true;
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
    alarmChanged = true;
  } else {
    // set next instance
    ScheduleAlarm();
  }
}

void AlarmController::SetRecurrence(RecurType recurrence) {
  if (alarm.recurrence != recurrence) {
    alarm.recurrence = recurrence;
    alarmChanged = true;
  }
}

void AlarmController::LoadSettingsFromFile() {
  lfs_file_t alarmFile;
  AlarmSettings alarmBuffer;

  if (fs.FileOpen(&alarmFile, "/.system/alarm.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[AlarmController] Failed to open alarm data file");
    return;
  }

  fs.FileRead(&alarmFile, reinterpret_cast<uint8_t*>(&alarmBuffer), sizeof(alarmBuffer));
  fs.FileClose(&alarmFile);
  if (alarmBuffer.version != alarmFormatVersion) {
    NRF_LOG_WARNING("[AlarmController] Loaded alarm settings has version %u instead of %u, discarding",
                    alarmBuffer.version,
                    alarmFormatVersion);
    return;
  }

  alarm = alarmBuffer;
  NRF_LOG_INFO("[AlarmController] Loaded alarm settings from file");
}

void AlarmController::SaveSettingsToFile() const {
  lfs_dir systemDir;
  if (fs.DirOpen("/.system", &systemDir) != LFS_ERR_OK) {
    fs.DirCreate("/.system");
  }
  fs.DirClose(&systemDir);
  lfs_file_t alarmFile;
  if (fs.FileOpen(&alarmFile, "/.system/alarm.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[AlarmController] Failed to open alarm data file for saving");
    return;
  }

  fs.FileWrite(&alarmFile, reinterpret_cast<const uint8_t*>(&alarm), sizeof(alarm));
  fs.FileClose(&alarmFile);
  NRF_LOG_INFO("[AlarmController] Saved alarm settings with format version %u to file", alarm.version);
}
