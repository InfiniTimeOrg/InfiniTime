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
#pragma once

#include <components/datetime/DateTimeController.h>
#include <components/heartrate/HeartRateController.h>
#include <components/motor/MotorController.h>
#include <components/alarm/AlarmController.h>
#include <drivers/Bma421.h>
#include <chrono>

namespace Pinetime::Controllers {
  class SleepTrackingController {
  private:
    using timepoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
    static constexpr uint8_t sleeptrackingSettingsFormatVersion = 1;
    static constexpr uint8_t maxSavedSessions = 5;
    static constexpr const char* settingsFileName = "sleeptracksettings.dat";
    static constexpr uint8_t wakeAlarmVibrationDurationStart = 50;
    static constexpr uint8_t maxSessionNameLength = 36;

    struct Settings {
      uint8_t version = sleeptrackingSettingsFormatVersion;
      uint8_t currentSession = 0;
      bool isTracking = false;

      struct Alarm {
        uint8_t hours = 7;
        uint8_t minutes = 0;
      } alarm;
    };

    struct SleepDataPoint {
      int16_t xDiffSum = 0;
      int16_t yDiffSum = 0;
      int16_t zDiffSum = 0;
      uint8_t heartRate = 0;
    };

  public:
    SleepTrackingController(FS& filesystem,
                            DateTime& datetimeController,
                            Pinetime::Drivers::Bma421& motionSensor,
                            HeartRateController& heartRateController,
                            MotorController& motorController);

    void Init(System::SystemTask* systemTask);

    void StartTracking();
    void StopTracking();
    void OnWakeAlarmTrigger();

    void OnMotionTrackingTimerTrigger();
    void OnHeartRateTrackingTimerTrigger();
    void OnStoreDataTimerTrigger();
    void OnGentleWakeupTimerTrigger();

    void ScheduleWakeAlarm();
    bool IsAlerting() const;
    bool IsTracking() const;
    void SaveDatapoint();

    Settings GetSettings();
    void SetSettings(const Settings& newSettings);
    void SaveSettings();

  private:
    void DismissWakeAlarm();
    void LoadSettings();
    void ClearTrackingFile();

    // Dependencies
    Settings settings;
    System::SystemTask* systemTask;
    FS& filesystem;
    DateTime& datetimeController;

    // Sleep tracking
    Pinetime::Drivers::Bma421& motionSensor;
    HeartRateController& heartRateController;
    TimerHandle_t motionTrackingTimer;
    TimerHandle_t heartRateTrackingTimer;
    TimerHandle_t storeDataTimer;
    SleepDataPoint currentDataPoint;
    Drivers::Bma421::Values previousValues;
    bool hasPreviousValues;

    // Wakeup
    TimerHandle_t wakeupAlarmTimer;
    TimerHandle_t gentleWakeupTimer;
    MotorController& motorController;
    uint16_t vibrationDurationMillis;
    bool isAlerting;
  };
}
