# include "components/infinisleep/InfiniSleepController.h"
#include "systemtask/SystemTask.h"
#include "task.h"
#include <chrono>
#include <libraries/log/nrf_log.h>

using namespace Pinetime::Controllers;
using namespace std::chrono_literals;

InfiniSleepController::InfiniSleepController(Controllers::DateTime& dateTimeController, Controllers::FS& fs, Controllers::HeartRateController& heartRateController)
  : dateTimeController {dateTimeController}, fs {fs}, heartRateController {heartRateController} {
}

namespace {
  void SetOffWakeAlarm(TimerHandle_t xTimer) {
    auto* controller = static_cast<Pinetime::Controllers::InfiniSleepController*>(pvTimerGetTimerID(xTimer));
    controller->SetOffWakeAlarmNow();
  }

  void SetOffGradualWake(TimerHandle_t xTimer) {
    auto* controller = static_cast<Pinetime::Controllers::InfiniSleepController*>(pvTimerGetTimerID(xTimer));
    if (controller->GetInfiniSleepSettings().graddualWake == false) {
      return;
    }
    controller->SetOffGradualWakeNow();
  }

  void SetOffTrackerUpdate(TimerHandle_t xTimer) {
    auto* controller = static_cast<Pinetime::Controllers::InfiniSleepController*>(pvTimerGetTimerID(xTimer));
    controller->UpdateTracker();
  }
}

void InfiniSleepController::Init(System::SystemTask* systemTask) {
    this->systemTask = systemTask;
    wakeAlarmTimer = xTimerCreate("WakeAlarm", 1, pdFALSE, this, SetOffWakeAlarm);
    gradualWakeTimer = xTimerCreate("GradualWake", 1, pdFALSE, this, SetOffGradualWake);

    LoadSettingsFromFile();
    if (wakeAlarm.isEnabled) {
        NRF_LOG_INFO("[InfiniSleepController] Loaded wake alarm was enabled, scheduling");
        ScheduleWakeAlarm();
    }
}

void InfiniSleepController::EnableTracker() {
  DisableTracker();
  NRF_LOG_INFO("[InfiniSleepController] Enabling tracker");
  isEnabled = true;
  trackerUpdateTimer = xTimerCreate("TrackerUpdate", 5 * configTICK_RATE_HZ, pdFALSE, this, SetOffTrackerUpdate);
  xTimerStart(trackerUpdateTimer, 0);
}

void InfiniSleepController::DisableTracker() {
  NRF_LOG_INFO("[InfiniSleepController] Disabling tracker");
  xTimerStop(trackerUpdateTimer, 0);
  isEnabled = false;
}

void InfiniSleepController::UpdateTracker() {
  NRF_LOG_INFO("[InfiniSleepController] Updating tracker");

  UpdateBPM();
  systemTask->PushMessage(System::Messages::SleepTrackerUpdate);

  xTimerStop(trackerUpdateTimer, 0);
  xTimerStart(trackerUpdateTimer, 0);
}

void InfiniSleepController::SaveWakeAlarm() {
    // verify is save needed
    if (wakeAlarmChanged) {
        SaveSettingsToFile();
    }
    wakeAlarmChanged = false;
}

void InfiniSleepController::SaveInfiniSleepSettings() {
    // verify is save needed
    if (settingsChanged) {
        SaveSettingsToFile();
    }
    settingsChanged = false;
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
    xTimerStop(gradualWakeTimer, 0);

    pushesLeftToStopWakeAlarm = PSUHES_TO_STOP_ALARM;

    gradualWakeStep = 9;

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
    int64_t secondsToWakeAlarm = std::chrono::duration_cast<std::chrono::seconds>(wakeAlarmTime - now).count();
    xTimerChangePeriod(wakeAlarmTimer, secondsToWakeAlarm * configTICK_RATE_HZ, 0);
    xTimerStart(wakeAlarmTimer, 0);

    // make sure graudal wake steps are possible
    while (gradualWakeStep != 0 && secondsToWakeAlarm <= gradualWakeSteps[gradualWakeStep-1]) {
      gradualWakeStep--;
    }

    // Calculate the period for the gradualWakeTimer
    if (infiniSleepSettings.graddualWake && gradualWakeStep != 0) {
      int64_t gradualWakePeriod = ((secondsToWakeAlarm - gradualWakeSteps[-1+gradualWakeStep--])) * (configTICK_RATE_HZ);
      xTimerChangePeriod(gradualWakeTimer, gradualWakePeriod, 0);
      xTimerStart(gradualWakeTimer, 0);
    }

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
  xTimerStop(gradualWakeTimer, 0);
  gradualWakeStep = 9;
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

void InfiniSleepController::SetOffGradualWakeNow() {
  //isGradualWakeAlerting = true;
  systemTask->PushMessage(System::Messages::SetOffGradualWake);
  // Calculate the period for the gradualWakeTimer
  if (infiniSleepSettings.graddualWake && gradualWakeStep != 0) {
    int64_t gradualWakePeriod = ((SecondsToWakeAlarm() - gradualWakeSteps[-1+gradualWakeStep--])) * (configTICK_RATE_HZ);
    xTimerChangePeriod(gradualWakeTimer, gradualWakePeriod, 0);
    xTimerStart(gradualWakeTimer, 0);
  }
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

/* Sleep Tracking Section */

void InfiniSleepController::UpdateBPM() {
  // Get the heart rate from the controller
  prevBpm = bpm;
  bpm = heartRateController.HeartRate();

  if(prevBpm != 0)
    rollingBpm = (rollingBpm + bpm) / 2;
  else
    rollingBpm = bpm;

  // Get the current time from DateTimeController
  int hours = dateTimeController.Hours();
  int minutes = dateTimeController.Minutes();
  int seconds = dateTimeController.Seconds();

  // Log the BPM and current time
  NRF_LOG_INFO("BPM: %d at %02d:%02d:%02d", rollingBpm, hours, minutes, seconds);

  // Write data to CSV
  const int motion = 0; // Placeholder for motion data
  std::tuple<int, int, int, int, int> data[1] = {std::make_tuple(hours, minutes, seconds, bpm, motion)};
  WriteDataCSV(TrackerDataFile, data, 1);
}

void InfiniSleepController::WriteDataCSV(const char* fileName, const std::tuple<int, int, int, int, int>* data, int dataSize) const {
  lfs_file_t file;
  int err = fs.FileOpen(&file, fileName, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND);
  if (err < 0) {
    // Handle error
    NRF_LOG_INFO("Error opening file: %d", err);
    return;
  }

  for (int i = 0; i < dataSize; ++i) {
    int hours, minutes, seconds, bpm, motion;
    std::tie(hours, minutes, seconds, bpm, motion) = data[i];
    char buffer[64];
    int len = snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d,%d,%d\n", hours, minutes, seconds, bpm, motion);
    err = fs.FileWrite(&file, reinterpret_cast<const uint8_t*>(buffer), len);
    if (err < 0) {
      // Handle error
      NRF_LOG_INFO("Error writing to file: %d", err);
      fs.FileClose(&file);

      return;
    }
  }

  fs.FileClose(&file);
}

// Clear data in CSV
void InfiniSleepController::ClearDataCSV(const char* filename) const {
  lfs_file_t file;
  int err = fs.FileOpen(&file, filename, LFS_O_WRONLY | LFS_O_TRUNC);
  if (err < 0) {
    // Handle error
    NRF_LOG_INFO("Error opening file: %d", err);
    return;
  }

  fs.FileClose(&file);
  NRF_LOG_INFO("CSV data cleared");
}

/* Sleep Tracking Section End */

void InfiniSleepController::LoadSettingsFromFile() {
  lfs_file_t wakeAlarmFile;
  WakeAlarmSettings wakeAlarmBuffer;

  if (fs.FileOpen(&wakeAlarmFile, "wakeAlarm.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[InfiniSleepController] Failed to open alarm data file");
    return;
  }

  fs.FileRead(&wakeAlarmFile, reinterpret_cast<uint8_t*>(&wakeAlarmBuffer), sizeof(wakeAlarmBuffer));
  fs.FileClose(&wakeAlarmFile);
  if (wakeAlarmBuffer.version != wakeAlarmFormatVersion) {
    NRF_LOG_WARNING("[InfiniSleepController] Loaded alarm settings has version %u instead of %u, discarding",
                    wakeAlarmBuffer.version,
                    wakeAlarmFormatVersion);
    return;
  }

  wakeAlarm = wakeAlarmBuffer;
  NRF_LOG_INFO("[InfiniSleepController] Loaded alarm settings from file");

  lfs_file_t infiniSleepSettingsFile;
  InfiniSleepSettings infiniSleepSettingsBuffer;

  if (fs.FileOpen(&infiniSleepSettingsFile, "infiniSleepSettings.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[InfiniSleepController] Failed to open InfiniSleep settings file");
    return;
  }

  fs.FileRead(&infiniSleepSettingsFile, reinterpret_cast<uint8_t*>(&infiniSleepSettingsBuffer), sizeof(infiniSleepSettingsBuffer));
  fs.FileClose(&infiniSleepSettingsFile);

  infiniSleepSettings = infiniSleepSettingsBuffer;
  NRF_LOG_INFO("[InfiniSleepController] Loaded InfiniSleep settings from file");
}

void InfiniSleepController::SaveSettingsToFile() const {
  // lfs_dir systemDir;
  // if (fs.DirOpen("/system/sleep", &systemDir) != LFS_ERR_OK) {
  //   fs.DirCreate("/system/sleep");
  // }
  // fs.DirClose(&systemDir);
  lfs_file_t alarmFile;
  if (fs.FileOpen(&alarmFile, "wakeAlarm.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[InfiniSleepController] Failed to open alarm data file for saving");
    return;
  }

  fs.FileWrite(&alarmFile, reinterpret_cast<const uint8_t*>(&wakeAlarm), sizeof(wakeAlarm));
  fs.FileClose(&alarmFile);
  NRF_LOG_INFO("[InfiniSleepController] Saved alarm settings with format version %u to file", wakeAlarm.version);

  lfs_file_t settingsFile;
  if (fs.FileOpen(&settingsFile, "infiniSleepSettings.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[InfiniSleepController] Failed to open  InfiniSleep settings file for saving");
    return;
  }

  fs.FileWrite(&settingsFile, reinterpret_cast<const uint8_t*>(&infiniSleepSettings), sizeof(infiniSleepSettings));
  fs.FileClose(&settingsFile);
  NRF_LOG_INFO("[InfiniSleepController] Saved InfiniSleep settings");
}