/*  Copyright (C) 2025 Asger Gitz-Johansen

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
#include "SleepTrackingController.h"

#include <cstdio>
#include <chrono>
#include <systemtask/SystemTask.h>
#include <libraries/log/nrf_log.h>

#define pdSEC_TO_TICKS(n) pdMS_TO_TICKS(n * 1000)
#define pdMIN_TO_TICKS(n) pdSEC_TO_TICKS(n * 60)
#ifndef MIN // Wrap in an ifndef becuase the simulator doesn't autoinclude it.
  #define MIN(a, b) ((a < b) ? (a) : (b))
#endif

namespace {
  void MotionTrackingTimerTrigger(TimerHandle_t xTimer) {
    static_cast<Pinetime::Controllers::SleepTrackingController*>(pvTimerGetTimerID(xTimer))->OnMotionTrackingTimerTrigger();
  }

  void HeartRateTrackingTimerTrigger(TimerHandle_t xTimer) {
    static_cast<Pinetime::Controllers::SleepTrackingController*>(pvTimerGetTimerID(xTimer))->OnHeartRateTrackingTimerTrigger();
  }

  void StoreDataTimerTrigger(TimerHandle_t xTimer) {
    static_cast<Pinetime::Controllers::SleepTrackingController*>(pvTimerGetTimerID(xTimer))->OnStoreDataTimerTrigger();
  }

  void GentleWakeupTimerTrigger(TimerHandle_t xTimer) {
    static_cast<Pinetime::Controllers::SleepTrackingController*>(pvTimerGetTimerID(xTimer))->OnGentleWakeupTimerTrigger();
  }

  void WakeAlarmTimerTrigger(TimerHandle_t xTimer) {
    static_cast<Pinetime::Controllers::SleepTrackingController*>(pvTimerGetTimerID(xTimer))->OnWakeAlarmTrigger();
  }
}

namespace Pinetime::Controllers {
  SleepTrackingController::SleepTrackingController(FS& filesystem,
                                                   DateTime& datetimeController,
                                                   Pinetime::Drivers::Bma421& motionSensor,
                                                   HeartRateController& heartRateController,
                                                   MotorController& motorController)
    : settings {},
      systemTask {nullptr},
      filesystem {filesystem},
      datetimeController {datetimeController},

      // Sleep tracking related
      motionSensor {motionSensor},
      heartRateController {heartRateController},
      motionTrackingTimer {},
      heartRateTrackingTimer {},
      storeDataTimer {},
      currentDataPoint {},
      previousValues {},
      hasPreviousValues {false},

      // Wakeup related
      wakeupAlarmTimer {},
      gentleWakeupTimer {},
      motorController {motorController},
      vibrationDurationMillis {wakeAlarmVibrationDurationStart},
      isAlerting {false} {
  }

  void SleepTrackingController::Init(System::SystemTask* systemTask) {
    this->systemTask = systemTask;
    motionTrackingTimer = xTimerCreate("sampleMotion", pdSEC_TO_TICKS(2), pdFALSE, this, MotionTrackingTimerTrigger);
    heartRateTrackingTimer = xTimerCreate("sampleHR", pdMIN_TO_TICKS(2), pdFALSE, this, HeartRateTrackingTimerTrigger);
    storeDataTimer = xTimerCreate("storeData", pdMIN_TO_TICKS(3), pdFALSE, this, StoreDataTimerTrigger);
    wakeupAlarmTimer = xTimerCreate("wakeupAlarm", 1, pdFALSE, this, WakeAlarmTimerTrigger);
    gentleWakeupTimer = xTimerCreate("gentleWakeup", pdSEC_TO_TICKS(10), pdFALSE, this, GentleWakeupTimerTrigger);
    LoadSettings();

    if (settings.isTracking) {
      StartTracking();
      NRF_LOG_INFO("[SleepTrackingController] Sleep tracking resumed");
    }
  }

  void SleepTrackingController::StartTracking() {
    // Reset tracking data.
    previousValues = {};
    hasPreviousValues = false;
    currentDataPoint = {};
    settings.isTracking = true;
    SaveSettings();
    ScheduleWakeAlarm();

    // Start tracking timers.
    xTimerStart(motionTrackingTimer, 0);
    xTimerStart(heartRateTrackingTimer, 0);
    xTimerStart(storeDataTimer, 0);
    NRF_LOG_INFO("[SleepTrackingController] Sleep tracking started");
  }

  void SleepTrackingController::StopTracking() {
    // Stop tracking timers.
    xTimerStop(motionTrackingTimer, 0);
    xTimerStop(heartRateTrackingTimer, 0);
    xTimerStop(storeDataTimer, 0);
    settings.isTracking = false;
    settings.currentSession = (settings.currentSession + 1) % 10;
    SaveSettings();
    DismissWakeAlarm();
    ClearTrackingFile();
    NRF_LOG_INFO("[SleepTrackingController] Sleep tracking stopped");
  }

  void SleepTrackingController::OnMotionTrackingTimerTrigger() {
    auto data = motionSensor.Process();
    if (hasPreviousValues) {
      currentDataPoint.xDiffSum += ABS(previousValues.x) - ABS(data.x);
      currentDataPoint.yDiffSum += ABS(previousValues.y) - ABS(data.y);
      currentDataPoint.zDiffSum += ABS(previousValues.z) - ABS(data.z);
    }
    previousValues = data;
    hasPreviousValues = true;
    xTimerStart(motionTrackingTimer, 0);
  }

  void SleepTrackingController::OnHeartRateTrackingTimerTrigger() {
    currentDataPoint.heartRate = heartRateController.HeartRate();
    xTimerStart(heartRateTrackingTimer, 0);
  }

  void SleepTrackingController::OnStoreDataTimerTrigger() {
    systemTask->PushMessage(Pinetime::System::Messages::OnSleepTrackingDataPoint);
  }

  void SleepTrackingController::SaveDatapoint() {
    lfs_file_t sleepDataFile;
    auto day = datetimeController.Day();
    auto month = datetimeController.Month();
    auto year = datetimeController.Year();
    auto hours = datetimeController.Hours();
    auto minutes = datetimeController.Minutes();
    auto seconds = datetimeController.Seconds();

    char filename[32] {};
    snprintf(filename, 32, "logs/sleep/session-%d.csv", settings.currentSession);

    // Ensure that the subdirectory exists.
    lfs_dir logdir {};
    if (filesystem.DirOpen("logs", &logdir) != LFS_ERR_OK) {
      filesystem.DirCreate("logs");
    }
    filesystem.DirClose(&logdir);
    if (filesystem.DirOpen("logs/sleep", &logdir) != LFS_ERR_OK) {
      filesystem.DirCreate("logs/sleep");
    }
    filesystem.DirClose(&logdir);

    if (filesystem.FileOpen(&sleepDataFile, filename, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND) < 0) {
      NRF_LOG_WARNING("[SleepTrackingController] Failed to open '%s' file", filename);
      xTimerStart(storeDataTimer, 0);
      return;
    }

    char buffer[64];
    auto len = snprintf(buffer,
                        sizeof(buffer),
                        "%04d-%02d-%02dT%02d:%02d:%02d,%d,%d,%d,%d\n",
                        year,
                        static_cast<uint8_t>(month),
                        day,
                        hours,
                        minutes,
                        seconds,
                        currentDataPoint.heartRate,
                        currentDataPoint.xDiffSum,
                        currentDataPoint.yDiffSum,
                        currentDataPoint.zDiffSum);
    filesystem.FileWrite(&sleepDataFile, reinterpret_cast<const uint8_t*>(buffer), len);
    filesystem.FileClose(&sleepDataFile);
    xTimerStart(storeDataTimer, 0);
  }

  void SleepTrackingController::DismissWakeAlarm() {
    isAlerting = false;
    xTimerStop(wakeupAlarmTimer, 0);
    xTimerStop(gentleWakeupTimer, 0);
    vibrationDurationMillis = wakeAlarmVibrationDurationStart;
    if (!isAlerting) {
      return;
    }
    motorController.StopRinging();
  }

  void SleepTrackingController::LoadSettings() {
    lfs_file_t settingsFile;
    Settings settingsBuffer;
    if (filesystem.FileOpen(&settingsFile, settingsFileName, LFS_O_RDONLY) < 0) {
      NRF_LOG_WARNING("[SleepTrackingController] Failed to open settings file");
      return;
    }

    filesystem.FileRead(&settingsFile, reinterpret_cast<uint8_t*>(&settingsBuffer), sizeof(settingsBuffer));
    filesystem.FileClose(&settingsFile);
    if (settingsBuffer.version != sleeptrackingSettingsFormatVersion) {
      NRF_LOG_WARNING("[SleepTrackingController] Loaded settings has version %u instead of %u, discarding",
                      settingsBuffer.version,
                      sleeptrackingSettingsFormatVersion);
      return;
    }

    settings = settingsBuffer;
    NRF_LOG_INFO("[SleepTrackingController] Loaded settings from file");
  }

  void SleepTrackingController::SaveSettings() {
    lfs_file_t settingsFile;
    if (filesystem.FileOpen(&settingsFile, settingsFileName, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) != LFS_ERR_OK) {
      NRF_LOG_WARNING("[SleepTrackingController] Failed to open settings file");
      return;
    }
    filesystem.FileWrite(&settingsFile, reinterpret_cast<uint8_t*>(&settings), sizeof(settings));
    filesystem.FileClose(&settingsFile);
    NRF_LOG_INFO("[SleepTrackingController] Saved settings to file");
  }

  void SleepTrackingController::OnWakeAlarmTrigger() {
    isAlerting = true;
    // Notify the system that the wake alarm is triggered, so we can show the alarm dismissal screen.
    systemTask->PushMessage(System::Messages::SetOffWakeAlarm);
  }

  void SleepTrackingController::OnGentleWakeupTimerTrigger() {
    // TODO: Also set intensity when motorcontroller supports it. (start low, end medium)
    motorController.RunForDuration(vibrationDurationMillis);
    vibrationDurationMillis = MIN(vibrationDurationMillis + 100, static_cast<uint16_t>(1000));
    xTimerStart(gentleWakeupTimer, 0);
  }

  void SleepTrackingController::ScheduleWakeAlarm() {
    // Determine the next time the alarm needs to go off and set the timer
    xTimerStop(wakeupAlarmTimer, 0);
    auto now = datetimeController.CurrentDateTime();
    auto ttAlarmTime = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(now));
    auto* tmAlarmTime = std::localtime(&ttAlarmTime);
    // If the time being set has already passed today,the alarm should be set for tomorrow
    if (settings.alarm.hours < datetimeController.Hours() ||
        (settings.alarm.hours == datetimeController.Hours() && settings.alarm.minutes <= datetimeController.Minutes())) {
      tmAlarmTime->tm_mday += 1;
      // tm_wday doesn't update automatically
      tmAlarmTime->tm_wday = (tmAlarmTime->tm_wday + 1) % 7;
    }
    tmAlarmTime->tm_hour = settings.alarm.hours;
    tmAlarmTime->tm_min = settings.alarm.minutes;
    tmAlarmTime->tm_sec = 0;
    tmAlarmTime->tm_isdst = -1; // use system timezone setting to determine DST
    // now can convert back to a time_point
    auto alarmTime = std::chrono::system_clock::from_time_t(std::mktime(tmAlarmTime));
    auto secondsToAlarm = std::chrono::duration_cast<std::chrono::seconds>(alarmTime - now).count();
    xTimerChangePeriod(wakeupAlarmTimer, secondsToAlarm * configTICK_RATE_HZ, 0);
    xTimerStart(wakeupAlarmTimer, 0);
    NRF_LOG_INFO("[SleepTrackingController] New alarm scheduled in %d seconds", secondsToAlarm);
  }

  void SleepTrackingController::ClearTrackingFile() {
    char filename[32] {};
    snprintf(filename, 32, "logs/sleep/session-%d.csv", settings.currentSession);
    lfs_info info;
    filesystem.Stat(filename, &info);
    if (info.size > 0) {
      lfs_file_t file;
      filesystem.FileOpen(&file, filename, LFS_O_CREAT | LFS_O_WRONLY | LFS_O_TRUNC); // NOTE: TRUNC = truncate
      filesystem.FileClose(&file);
    }
  }

  SleepTrackingController::Settings SleepTrackingController::GetSettings() {
    return settings;
  }

  void SleepTrackingController::SetSettings(const Settings& newSettings) {
    settings = newSettings;
  }

  bool SleepTrackingController::IsAlerting() const {
    return isAlerting;
  }

  bool SleepTrackingController::IsTracking() const {
    return settings.isTracking;
  }
}
