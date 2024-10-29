# include "components/infinisleep/InfiniSleepController.h"
#include "systemtask/SystemTask.h"
#include "task.h"
#include <chrono>
#include <libraries/log/nrf_log.h>

using namespace Pinetime::Controllers;
using namespace std::chrono_literals;

InfiniSleepController::InfiniSleepController(Controllers::DateTime& dateTimeController, Controllers::FS& fs)
  : dateTimeController {dateTimeController}, fs {fs} {
}

namespace {
  void SetOffWakeAlarm(TimerHandle_t xTimer) {
    auto* controller = static_cast<Pinetime::Controllers::InfiniSleepController*>(pvTimerGetTimerID(xTimer));
    controller->SetOffWakeAlarmNow();
  }
}

void InfiniSleepController::Init(System::SystemTask* systemTask) {
    this->systemTask = systemTask;
    wakeAlarmTimer = xTimerCreate("WakeAlarm", 1, pdFALSE, this, SetOffWakeAlarm);
    LoadSettingsFromFile();
    if (wakeAlarm.isEnabled) {
        NRF_LOG_INFO("[InfiniSleepController] Loaded wake alarm was enabled, scheduling");
        ScheduleWakeAlarm();
    }
}

void InfiniSleepController::SaveWakeAlarm() {
    // verify is save needed
    if (wakeAlarmChanged) {
        SaveSettingsToFile();
    }
    wakeAlarmChanged = false;
}

void InfiniSleepController::SetWakeAlarmTime(uint8_t wakeAlarmHr, uint8_t wakeAlarmMin) {
    if (wakeAlarm.hours == wakeAlarmHr && wakeAlarm.minutes == wakeAlarmMin) {
        return;
    }
    wakeAlarm.hours = wakeAlarmHr;
    wakeAlarm.minutes = wakeAlarmMin;
    wakeAlarmChanged = true;
}

void InfiniSleepController::ScheduleWakeAlarm() {
    // Determine the next time the wake alarm needs to go off and set the timer
    xTimerStop(wakeAlarmTimer, 0);

    auto now = dateTimeController.CurrentDateTime();
    wakeAlarmTime = now;
    time_t ttWakeAlarmTime = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(wakeAlarmTime));
    tm* tmWakeAlarmTime = std::localtime(&ttWakeAlarmTime);

    // If the time being set has already passed today, the wake alarm should be set for tomorrow
    if (wakeAlarm.hours < dateTimeController.Hours() ||
        (wakeAlarm.hours == dateTimeController.Hours() && wakeAlarm.minutes <= dateTimeController.Minutes())) {
      tmWakeAlarmTime->tm_mday += 1;
      // tm_wday doesn't update automatically
      tmWakeAlarmTime->tm_wday = (tmWakeAlarmTime->tm_wday + 1) % 7;
    }

    tmWakeAlarmTime->tm_hour = wakeAlarm.hours;
    tmWakeAlarmTime->tm_min = wakeAlarm.minutes;
    tmWakeAlarmTime->tm_sec = 0;

    // if alarm is in weekday-only mode, make sure it shifts to the next weekday
    if (wakeAlarm.recurrence == RecurType::Weekdays) {
      if (tmWakeAlarmTime->tm_wday == 0) {// Sunday, shift 1 day
        tmWakeAlarmTime->tm_mday += 1;
      } else if (tmWakeAlarmTime->tm_wday == 6) { // Saturday, shift 2 days
        tmWakeAlarmTime->tm_mday += 2;
      }
    }
    tmWakeAlarmTime->tm_isdst = -1; // use system timezone setting to determine DST

    // now can convert back to a time_point
    wakeAlarmTime = std::chrono::system_clock::from_time_t(std::mktime(tmWakeAlarmTime));
    auto secondsToWakeAlarm = std::chrono::duration_cast<std::chrono::seconds>(wakeAlarmTime - now).count();
    xTimerChangePeriod(wakeAlarmTimer, secondsToWakeAlarm * configTICK_RATE_HZ, 0);
    xTimerStart(wakeAlarmTimer, 0);

    if (!wakeAlarm.isEnabled) {
        wakeAlarm.isEnabled = true;
        wakeAlarmChanged = true;
    }
}

uint32_t InfiniSleepController::SecondsToWakeAlarm() const {
    return std::chrono::duration_cast<std::chrono::seconds>(wakeAlarmTime - dateTimeController.CurrentDateTime()).count();
}

void InfiniSleepController::DisableWakeAlarm() {
  xTimerStop(wakeAlarmTimer, 0);
  isAlerting = false;
  if (wakeAlarm.isEnabled) {
    wakeAlarm.isEnabled = false;
    wakeAlarmChanged = true;
  }
}

void InfiniSleepController::SetOffWakeAlarmNow() {
  isAlerting = true;
  systemTask->PushMessage(System::Messages::SetOffWakeAlarm);
}

void InfiniSleepController::StopAlerting() {
  isAlerting = false;
  // Disable the alarm unless it is recurring
  if (wakeAlarm.recurrence == RecurType::None) {
    wakeAlarm.isEnabled = false;
    wakeAlarmChanged = true;
  } else {
    // Schedule the alarm for the next day
    ScheduleWakeAlarm();
  }
}

void InfiniSleepController::SetRecurrence(RecurType recurrence) {
  if (wakeAlarm.recurrence == recurrence) {
    return;
  }
  wakeAlarm.recurrence = recurrence;
  wakeAlarmChanged = true;
}

void InfiniSleepController::LoadSettingsFromFile() {
  lfs_file_t wakeAlarmFile;
  WakeAlarmSettings wakeAlarmBuffer;

  if (fs.FileOpen(&wakeAlarmFile, "/.system/sleep/wakeAlarm.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[AlarmController] Failed to open alarm data file");
    return;
  }

  fs.FileRead(&wakeAlarmFile, reinterpret_cast<uint8_t*>(&wakeAlarmBuffer), sizeof(wakeAlarmBuffer));
  fs.FileClose(&wakeAlarmFile);
  if (wakeAlarmBuffer.version != wakeAlarmFormatVersion) {
    NRF_LOG_WARNING("[AlarmController] Loaded alarm settings has version %u instead of %u, discarding",
                    wakeAlarmBuffer.version,
                    wakeAlarmFormatVersion);
    return;
  }

  wakeAlarm = wakeAlarmBuffer;
  NRF_LOG_INFO("[AlarmController] Loaded alarm settings from file");
}

void InfiniSleepController::SaveSettingsToFile() const {
  lfs_dir systemDir;
  if (fs.DirOpen("/.system/sleep", &systemDir) != LFS_ERR_OK) {
    fs.DirCreate("/.system/sleep");
  }
  fs.DirClose(&systemDir);
  lfs_file_t alarmFile;
  if (fs.FileOpen(&alarmFile, "/.system/sleep/wakeAlarm.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[AlarmController] Failed to open alarm data file for saving");
    return;
  }

  fs.FileWrite(&alarmFile, reinterpret_cast<const uint8_t*>(&wakeAlarm), sizeof(wakeAlarm));
  fs.FileClose(&alarmFile);
  NRF_LOG_INFO("[AlarmController] Saved alarm settings with format version %u to file", wakeAlarm.version);
}