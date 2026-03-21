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
  // Initialize all alarms with defaults
  for (uint8_t i = 0; i < MaxAlarms; i++) {
    alarms[i].version = alarmFormatVersion;
    alarms[i].hours = 6 + i;
    alarms[i].minutes = 0;
    alarms[i].recurrence = RecurType::Weekdays;
    alarms[i].isEnabled = false;
  }
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
  // Check if any alarms are enabled and schedule the next one
  for (uint8_t i = 0; i < MaxAlarms; i++) {
    if (alarms[i].isEnabled) {
      NRF_LOG_INFO("[AlarmController] Found enabled alarm %u, scheduling next alarm", i);
      ScheduleAlarm();
      break;
    }
  }
}

void AlarmController::SaveAlarm() {
  // verify if it is necessary to save
  if (alarmChanged) {
    SaveSettingsToFile();
  }
  alarmChanged = false;
}

void AlarmController::SetAlarmTime(uint8_t index, uint8_t alarmHr, uint8_t alarmMin) {
  if (index >= MaxAlarms) {
    return;
  }
  if (alarms[index].hours == alarmHr && alarms[index].minutes == alarmMin) {
    return;
  }
  alarms[index].hours = alarmHr;
  alarms[index].minutes = alarmMin;
  alarmChanged = true;
}

void AlarmController::ScheduleAlarm() {
  // Determine the next alarm to schedule and set the timer
  xTimerStop(alarmTimer, 0);

  nextAlarmIndex = CalculateNextAlarm();
  if (nextAlarmIndex >= MaxAlarms) {
    // No enabled alarms found
    return;
  }

  auto now = dateTimeController.CurrentDateTime();
  alarmTime = now;
  time_t ttAlarmTime = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(alarmTime));
  tm* tmAlarmTime = std::localtime(&ttAlarmTime);

  // If the time being set has already passed today,the alarm should be set for tomorrow
  if (alarms[nextAlarmIndex].hours < dateTimeController.Hours() ||
      (alarms[nextAlarmIndex].hours == dateTimeController.Hours() && alarms[nextAlarmIndex].minutes <= dateTimeController.Minutes())) {
    tmAlarmTime->tm_mday += 1;
    // tm_wday doesn't update automatically
    tmAlarmTime->tm_wday = (tmAlarmTime->tm_wday + 1) % 7;
  }

  tmAlarmTime->tm_hour = alarms[nextAlarmIndex].hours;
  tmAlarmTime->tm_min = alarms[nextAlarmIndex].minutes;
  tmAlarmTime->tm_sec = 0;

  // if alarm is in weekday-only mode, make sure it shifts to the next weekday
  if (alarms[nextAlarmIndex].recurrence == RecurType::Weekdays) {
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
}

uint32_t AlarmController::SecondsToAlarm() const {
  return std::chrono::duration_cast<std::chrono::seconds>(alarmTime - dateTimeController.CurrentDateTime()).count();
}

void AlarmController::DisableAlarm(uint8_t index) {
  if (index >= MaxAlarms) {
    return;
  }
  if (alarms[index].isEnabled) {
    alarms[index].isEnabled = false;
    alarmChanged = true;
    ScheduleAlarm();
  }
}

void AlarmController::SetEnabled(uint8_t index, bool enabled) {
  if (index >= MaxAlarms) {
    return;
  }
  if (alarms[index].isEnabled != enabled) {
    alarms[index].isEnabled = enabled;
    alarmChanged = true;
    ScheduleAlarm();
  }
}

void AlarmController::SetOffAlarmNow() {
  isAlerting = true;
  alertingAlarmIndex = nextAlarmIndex;
  systemTask->PushMessage(System::Messages::SetOffAlarm);
}

void AlarmController::StopAlerting() {
  isAlerting = false;
  // Disable alarm unless it is recurring
  if (alarms[alertingAlarmIndex].recurrence == RecurType::None) {
    alarms[alertingAlarmIndex].isEnabled = false;
    alarmChanged = true;
  }
  ScheduleAlarm();
}

void AlarmController::SetRecurrence(uint8_t index, RecurType recurrence) {
  if (index >= MaxAlarms) {
    return;
  }
  if (alarms[index].recurrence != recurrence) {
    alarms[index].recurrence = recurrence;
    alarmChanged = true;
  }
}

void AlarmController::LoadSettingsFromFile() {
  lfs_file_t alarmFile;

  if (fs.FileOpen(&alarmFile, "/.system/alarm.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[AlarmController] Failed to open alarm data file");
    return;
  }

  // Read version byte to determine format
  uint8_t version;
  fs.FileRead(&alarmFile, &version, sizeof(version));
  fs.FileSeek(&alarmFile, 0);

  if (version == 1) {
    // Migrate from old single-alarm format
    NRF_LOG_INFO("[AlarmController] Migrating from version 1 to version 2");
    AlarmSettings oldAlarm;
    fs.FileRead(&alarmFile, reinterpret_cast<uint8_t*>(&oldAlarm), sizeof(oldAlarm));
    fs.FileClose(&alarmFile);

    // Copy old alarm to first slot
    alarms[0] = oldAlarm;
    alarms[0].version = alarmFormatVersion;

    // Initialize other alarms with defaults
    for (uint8_t i = 1; i < MaxAlarms; i++) {
      alarms[i].version = alarmFormatVersion;
      alarms[i].hours = 6 + i;
      alarms[i].minutes = 0;
      alarms[i].recurrence = RecurType::Weekdays;
      alarms[i].isEnabled = false;
    }

    alarmChanged = true;
    NRF_LOG_INFO("[AlarmController] Migrated alarm settings from version 1");
  } else if (version == alarmFormatVersion) {
    // Read new multi-alarm format
    fs.FileRead(&alarmFile, reinterpret_cast<uint8_t*>(alarms.data()), sizeof(alarms));
    fs.FileClose(&alarmFile);
    NRF_LOG_INFO("[AlarmController] Loaded %u alarms from file", MaxAlarms);
  } else {
    NRF_LOG_WARNING("[AlarmController] Unknown alarm version %u, using defaults", version);
    fs.FileClose(&alarmFile);
  }
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

  fs.FileWrite(&alarmFile, reinterpret_cast<const uint8_t*>(alarms.data()), sizeof(alarms));
  fs.FileClose(&alarmFile);
  NRF_LOG_INFO("[AlarmController] Saved %u alarms with format version %u to file", MaxAlarms, alarmFormatVersion);
}

uint8_t AlarmController::CalculateNextAlarm() const {
  auto now = dateTimeController.CurrentDateTime();
  uint32_t minSecondsToAlarm = UINT32_MAX;
  uint8_t nextIndex = MaxAlarms; // Invalid index means no alarm found

  for (uint8_t i = 0; i < MaxAlarms; i++) {
    if (!alarms[i].isEnabled) {
      continue;
    }

    // Calculate seconds to this alarm
    auto alarmTimePoint = now;
    time_t ttAlarmTime =
      std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(alarmTimePoint));
    tm* tmAlarmTime = std::localtime(&ttAlarmTime);

    // If the time has already passed today, schedule for tomorrow
    if (alarms[i].hours < dateTimeController.Hours() ||
        (alarms[i].hours == dateTimeController.Hours() && alarms[i].minutes <= dateTimeController.Minutes())) {
      tmAlarmTime->tm_mday += 1;
      tmAlarmTime->tm_wday = (tmAlarmTime->tm_wday + 1) % 7;
    }

    tmAlarmTime->tm_hour = alarms[i].hours;
    tmAlarmTime->tm_min = alarms[i].minutes;
    tmAlarmTime->tm_sec = 0;

    // Handle weekday-only mode
    if (alarms[i].recurrence == RecurType::Weekdays) {
      if (tmAlarmTime->tm_wday == 0) { // Sunday, shift 1 day
        tmAlarmTime->tm_mday += 1;
      } else if (tmAlarmTime->tm_wday == 6) { // Saturday, shift 2 days
        tmAlarmTime->tm_mday += 2;
      }
    }
    tmAlarmTime->tm_isdst = -1;

    auto thisAlarmTime = std::chrono::system_clock::from_time_t(std::mktime(tmAlarmTime));
    auto secondsToThisAlarm = std::chrono::duration_cast<std::chrono::seconds>(thisAlarmTime - now).count();

    if (secondsToThisAlarm > 0 && static_cast<uint32_t>(secondsToThisAlarm) < minSecondsToAlarm) {
      minSecondsToAlarm = static_cast<uint32_t>(secondsToThisAlarm);
      nextIndex = i;
    }
  }

  return nextIndex;
}
