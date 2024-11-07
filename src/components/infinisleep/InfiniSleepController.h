#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <cstdint>
#include "components/datetime/DateTimeController.h"
#include "components/fs/FS.h"
#include "components/heartrate/HeartRateController.h"

#include <chrono>

#define PSUHES_TO_STOP_ALARM 5
#define TRACKER_UPDATE_INTERVAL_MINS 5

namespace Pinetime {
    namespace System {
        class SystemTask;
    }

    namespace Controllers {
        class InfiniSleepController {
            public:
                InfiniSleepController(Controllers::DateTime& dateTimeCOntroller, Controllers::FS& , Controllers::HeartRateController& heartRateController);

                void Init(System::SystemTask* systemTask);
                void SaveWakeAlarm();
                void SaveInfiniSleepSettings();
                void SetWakeAlarmTime(uint8_t wakeAlarmHr, uint8_t wakeAlarmMin);
                void ScheduleWakeAlarm();
                void DisableWakeAlarm();
                void SetOffWakeAlarmNow();
                void SetOffGradualWakeNow();
                uint32_t SecondsToWakeAlarm() const;
                void StopAlerting();
                enum class RecurType { None, Daily, Weekdays };

                uint8_t pushesLeftToStopWakeAlarm = PSUHES_TO_STOP_ALARM;

                bool isSnoozing = false;
                uint8_t preSnoozeMinutes = 255;
                uint8_t preSnnoozeHours = 255;

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

                RecurType Recurrence() const {
                    return wakeAlarm.recurrence;
                }

                void SetRecurrence(RecurType recurrence);

                bool BodyTrackingEnabled() const {
                    return infiniSleepSettings.bodyTracking;
                }

                void SetBodyTrackingEnabled(bool enabled) {
                    infiniSleepSettings.bodyTracking = enabled;
                }

                bool HeartRateTrackingEnabled() const {
                    return infiniSleepSettings.heartRateTracking;
                }

                void SetHeartRateTrackingEnabled(bool enabled) {
                    infiniSleepSettings.heartRateTracking = enabled;
                }

                bool GradualWakeEnabled() const {
                    return infiniSleepSettings.graddualWake;
                }

                void SetGradualWakeEnabled(bool enabled) {
                    infiniSleepSettings.graddualWake = enabled;
                }

                bool SmartAlarmEnabled() const {
                    return infiniSleepSettings.smartAlarm;
                }

                void SetSmartAlarmEnabled(bool enabled) {
                    infiniSleepSettings.smartAlarm = enabled;
                }

                void SetSettingsChanged() {
                    settingsChanged = true;
                }

                // Versions 255 is reserved for now, so the version field can be made
                // bigger, should it ever be needed.
                static constexpr uint8_t wakeAlarmFormatVersion = 1;

                struct WakeAlarmSettings {
                    uint8_t version = wakeAlarmFormatVersion;
                    uint8_t hours = 7;
                    uint8_t minutes = 0;
                    RecurType recurrence = RecurType::None;
                    bool isEnabled = false;
                };

                uint16_t gradualWakeSteps[9] = {30, 60, 90, 120, 180, 240, 300, 350, 600}; // In seconds


                WakeAlarmSettings GetWakeAlarm() const {
                    return wakeAlarm;
                }

                struct InfiniSleepSettings {
                    bool bodyTracking = false;
                    bool heartRateTracking = true;
                    bool graddualWake = false;
                    bool smartAlarm = false;
                };

                InfiniSleepSettings GetInfiniSleepSettings() const {
                    return infiniSleepSettings;
                }

                bool ToggleTracker() {
                    if (isEnabled) {
                        DisableTracker();
                    } else {
                        EnableTracker();
                    }
                    return isEnabled;
                }

                bool IsTrackerEnabled() const {
                    return isEnabled;
                }

                uint8_t GetCurrentHour() {
                    return dateTimeController.Hours();
                }

                uint8_t GetCurrentMinute() {
                    return dateTimeController.Minutes();
                }

                //int64_t secondsToWakeAlarm = 0;

                int bpm = 0;
                int prevBpm = 0;
                int rollingBpm = 0;

                void UpdateBPM();

            private:

                bool isAlerting = false;
                bool isGradualWakeAlerting = false;
                uint8_t gradualWakeStep = 9; // used to keep track of which step to use, in position form not idex
                bool wakeAlarmChanged = false;
                bool isEnabled = false;
                bool settingsChanged = false;

                InfiniSleepSettings infiniSleepSettings;

                Controllers::DateTime& dateTimeController;
                Controllers::FS& fs;
                Controllers::HeartRateController& heartRateController;
                System::SystemTask* systemTask = nullptr;
                TimerHandle_t wakeAlarmTimer;
                TimerHandle_t gradualWakeTimer;
                TimerHandle_t trackerUpdateTimer;
                WakeAlarmSettings wakeAlarm;
                std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> wakeAlarmTime;

                void LoadSettingsFromFile();
                void SaveSettingsToFile() const;

                // For File IO
                void WriteDataCSV(const char* fileName, const std::tuple<int, int, int, int, int>* data, int dataSize) const;
                void ClearDataCSV(const char* fileName) const;
        };
    }

}