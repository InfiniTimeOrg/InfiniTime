#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <cstdint>
#include "components/datetime/DateTimeController.h"
#include "components/fs/FS.h"
#include "components/heartrate/HeartRateController.h"
#include "components/alarm/AlarmController.h"

#include <chrono>

#define SNOOZE_MINUTES               3
#define PUSHES_TO_STOP_ALARM         5
#define TRACKER_UPDATE_INTERVAL_MINS 5
#define TRACKER_DATA_FILE_NAME       "SleepTracker_Data.csv"
#define PREV_SESSION_DATA_FILE_NAME  "SleepTracker_PrevSession.csv"
#define SLEEP_CYCLE_DURATION         90 // sleep cycle duration in minutes
#define DESIRED_CYCLES               5  // desired number of sleep cycles
#define PUSHES_TO_STOP_ALARM_TIMEOUT 2  // in seconds

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    namespace InfiniSleepControllerTypes {
      // Struct for sessions
      struct SessionData {
        uint8_t day = 0;
        uint8_t month = 0;
        uint16_t year = 0;

        uint8_t startTimeHours = 0;
        uint8_t startTimeMinutes = 0;
        uint8_t endTimeHours = 0;
        uint8_t endTimeMinutes = 0;
      };
    }

    class InfiniSleepController {
    public:
      InfiniSleepController(Controllers::DateTime& dateTimeCOntroller,
                            Controllers::FS&,
                            Controllers::HeartRateController& heartRateController,
                            Controllers::BrightnessController& brightnessController);

      void Init(System::SystemTask* systemTask);
      void SaveWakeAlarm();
      void SaveInfiniSleepSettings();
      void SetWakeAlarmTime(uint8_t wakeAlarmHr, uint8_t wakeAlarmMin);
      void ScheduleWakeAlarm();
      void DisableWakeAlarm();
      void EnableWakeAlarm();
      void SetOffWakeAlarmNow();
      void SetOffGradualWakeNow();
      void UpdateGradualWake();
      uint32_t SecondsToWakeAlarm() const;
      void StopAlerting();

      uint8_t pushesLeftToStopWakeAlarm = PUSHES_TO_STOP_ALARM;

      bool isSnoozing = false;
      uint8_t preSnoozeMinutes = 255;
      uint8_t preSnnoozeHours = 255;

      InfiniSleepControllerTypes::SessionData prevSessionData;

      void SetPreSnoozeTime() {
        if (preSnoozeMinutes != 255 || preSnnoozeHours != 255) {
          return;
        }
        preSnoozeMinutes = wakeAlarm.minutes;
        preSnnoozeHours = wakeAlarm.hours;
      }

      void RestorePreSnoozeTime() {
        if (preSnoozeMinutes == 255 || preSnnoozeHours == 255) {
          return;
        }
        wakeAlarm.minutes = preSnoozeMinutes;
        wakeAlarm.hours = preSnnoozeHours;
        preSnoozeMinutes = 255;
        preSnnoozeHours = 255;
      }

      uint8_t Hours() const {
        return wakeAlarm.hours;
      }

      uint8_t Minutes() const {
        return wakeAlarm.minutes;
      }

      bool IsAlerting() const {
        return isAlerting;
      }

      bool IsEnabled() const {
        return isEnabled;
      }

      void EnableTracker();
      void DisableTracker();
      void UpdateTracker();

      void SetSettingsChanged() {
        settingsChanged = true;
      }

      // Versions 255 is reserved for now, so the version field can be made
      // bigger, should it ever be needed.
      static constexpr uint8_t wakeAlarmFormatVersion = 1;

      struct WakeAlarmSettings {
        static constexpr uint8_t version = wakeAlarmFormatVersion;
        uint8_t hours = 7;
        uint8_t minutes = 0;
        AlarmController::RecurType recurrence = AlarmController::RecurType::Daily;
        bool isEnabled = false;
      };

      WakeAlarmSettings wakeAlarm;

      // Dertermine the steps for the gradual wake alarm, the corresponding vibration durations determine the power of the vibration
      static constexpr uint16_t gradualWakeSteps[9] = {30, 60, 90, 120, 180, 240, 300, 350, 600}; // In seconds

      uint8_t gradualWakeStep = 9; // used to keep track of which step to use, in position form not idex

      uint16_t GetSleepCycles() const {
        return (GetTotalSleep() * 100 / infiniSleepSettings.sleepCycleDuration);
      }

      uint16_t GetTotalSleep() const {
        uint8_t endHours = IsEnabled() ? GetCurrentHour() : prevSessionData.endTimeHours;
        uint8_t endMinutes = IsEnabled() ? GetCurrentMinute() : prevSessionData.endTimeMinutes;

        // Calculate total minutes for start and end times
        uint16_t startTotalMinutes = prevSessionData.startTimeHours * 60 + prevSessionData.startTimeMinutes;
        uint16_t endTotalMinutes = endHours * 60 + endMinutes;

        // If end time is before start time, add 24 hours to end time (handle crossing midnight)
        if (endTotalMinutes < startTotalMinutes) {
          endTotalMinutes += 24 * 60;
        }

        uint16_t sleepMinutes = endTotalMinutes - startTotalMinutes;

        return sleepMinutes;
      }

      uint16_t GetSuggestedSleepTime() const {
        return infiniSleepSettings.desiredCycles * infiniSleepSettings.sleepCycleDuration;
      }

      WakeAlarmSettings GetWakeAlarm() const {
        return wakeAlarm;
      }

      struct InfiniSleepSettings {
        bool bodyTracking = false;
        bool heartRateTracking = true;
        bool graddualWake = false;
        bool smartAlarm = false;
        uint8_t sleepCycleDuration = SLEEP_CYCLE_DURATION;
        uint8_t desiredCycles = DESIRED_CYCLES;
        uint8_t motorStrength = 100;
        bool naturalWake = false;
        uint8_t pushesToStopAlarm = PUSHES_TO_STOP_ALARM;
      };

      InfiniSleepSettings infiniSleepSettings;

      InfiniSleepSettings GetInfiniSleepSettings() const {
        return infiniSleepSettings;
      }

      BrightnessController::Levels prevBrightnessLevel;

      bool ToggleTracker() {
        if (isEnabled) {
          prevSessionData.endTimeHours = GetCurrentHour();
          prevSessionData.endTimeMinutes = GetCurrentMinute();
          SavePrevSessionData();
          DisableTracker();
        } else {
          // ClearDataCSV(TRACKER_DATA_FILE_NAME);
          prevSessionData.endTimeHours = 255;
          prevSessionData.endTimeMinutes = 255;
          prevSessionData.startTimeHours = GetCurrentHour();
          prevSessionData.startTimeMinutes = GetCurrentMinute();
          prevSessionData.day = dateTimeController.Day();
          prevSessionData.month = static_cast<uint8_t>(dateTimeController.Month());
          prevSessionData.year = dateTimeController.Year();
          EnableTracker();
        }
        return isEnabled;
      }

      bool IsTrackerEnabled() const {
        return isEnabled;
      }

      uint8_t GetCurrentHour() const {
        return dateTimeController.Hours();
      }

      uint8_t GetCurrentMinute() const {
        return dateTimeController.Minutes();
      }

      void UpdateBPM();

      uint8_t GetGradualWakeStep() const {
        return (9 - gradualWakeStep) + 1;
      }

      BrightnessController& GetBrightnessController() {
        return brightnessController;
      }

    private:
      bool isAlerting = false;
      bool isGradualWakeAlerting = false;
      bool wakeAlarmChanged = false;
      bool isEnabled = false;
      bool settingsChanged = false;

      // uint8_t bpm = 0;
      // uint8_t prevBpm = 0;
      // uint8_t rollingBpm = 0;

      Controllers::DateTime& dateTimeController;
      Controllers::FS& fs;
      Controllers::HeartRateController& heartRateController;
      Controllers::BrightnessController& brightnessController;
      System::SystemTask* systemTask = nullptr;
      TimerHandle_t wakeAlarmTimer;
      TimerHandle_t gradualWakeTimer;
      TimerHandle_t trackerUpdateTimer;
      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> wakeAlarmTime;

      void LoadSettingsFromFile();
      void SaveSettingsToFile() const;
      void LoadPrevSessionData();
      void SavePrevSessionData() const;

      // For File IO
      // void WriteDataCSV(const char* fileName, const std::tuple<int, int, int, int, int>* data, int dataSize) const;
      // void ClearDataCSV(const char* fileName) const;
    };
  }

}