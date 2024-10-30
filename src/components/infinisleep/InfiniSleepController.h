#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <cstdint>
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
    namespace System {
        class SystemTask;
    }

    namespace Controllers {
        class InfiniSleepController {
            public:
                InfiniSleepController(Controllers::DateTime& dateTimeCOntroller, Controllers::FS& fs);

                void Init(System::SystemTask* systemTask);
                void SaveWakeAlarm();
                void SetWakeAlarmTime(uint8_t wakeAlarmHr, uint8_t wakeAlarmMin);
                void ScheduleWakeAlarm();
                void DisableWakeAlarm();
                void SetOffWakeAlarmNow();
                uint32_t SecondsToWakeAlarm() const;
                void StopAlerting();
                enum class RecurType { None, Daily, Weekdays };

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
                    return wakeAlarm.isEnabled;
                }

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

            private:
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

                struct InfiniSleepSettings {
                    bool bodyTracking = false;
                    bool heartRateTracking = true;
                    bool graddualWake = false;
                    bool smartAlarm = false;
                };

                bool isAlerting = false;
                bool wakeAlarmChanged = false;
                bool isEnabled = false;

                InfiniSleepSettings infiniSleepSettings;

                Controllers::DateTime& dateTimeController;
                Controllers::FS& fs;
                System::SystemTask* systemTask = nullptr;
                TimerHandle_t wakeAlarmTimer;
                WakeAlarmSettings wakeAlarm;
                std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> wakeAlarmTime;

                void LoadSettingsFromFile();
                void SaveSettingsToFile() const;
        };
    }

}