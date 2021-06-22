#pragma once

#include "Screen.h"
#include "components/settings/Settings.h"
#include "components/datetime/DateTime.h"
#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class DateTimeController;
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
  }
  namespace Applications {
    class DisplayApp;

    namespace Screens {
      class WatchFaceBase : public Screen {
      public:
        struct DateState {
          uint16_t year;
          DateTime::Months month;
          uint8_t day;
          DateTime::Days dayOfWeek;

          bool operator!=(DateState const& rhs) {
            return year != rhs.year || month != rhs.month || day != rhs.day || dayOfWeek != rhs.dayOfWeek;
          }
        };

        struct TimeState {
          uint8_t hour;
          uint8_t minute;
          uint8_t second;

          bool operator!=(TimeState const& rhs) {
            return hour != rhs.hour || minute != rhs.minute || second != rhs.second;
          }
        };

        struct BatteryState {
          uint8_t percentRemaining;
          bool charging;
          bool powerPresent;

          bool operator!=(BatteryState const& rhs) {
            return percentRemaining != rhs.percentRemaining || charging != rhs.charging || powerPresent != rhs.powerPresent;
          }
        };

        struct BleState {
          bool connected;

          bool operator!=(BleState const& rhs) {
            return connected != rhs.connected;
          }
        };

        struct NotificationState {
          bool newNotificationsAvailable;

          bool operator!=(NotificationState const& rhs) {
            return newNotificationsAvailable != rhs.newNotificationsAvailable;
          }
        };

        struct HeartRateState {
          bool running;
          uint8_t rate;

          bool operator!=(HeartRateState const& rhs) {
            return running != rhs.running || rate != rhs.rate;
          }
        };

        WatchFaceBase(Controllers::Settings::ClockFace face,
                      DisplayApp* app,
                      Controllers::Settings& settingsController,
                      Controllers::DateTimeController const& dateTimeController,
                      Controllers::Battery const& batteryController,
                      Controllers::Ble const& bleController,
                      Controllers::NotificationManager const& notificationManager,
                      Controllers::HeartRateController const& heartRateController);

        virtual ~WatchFaceBase() = default;

        Controllers::Settings::ClockType GetClockType() const;

        void UpdateDate();
        void UpdateTime();
        DirtyValue<DateState> const& GetDate() const {
          return date;
        }
        DirtyValue<TimeState> const& GetTime() const {
          return time;
        }
        DirtyValue<DateState> const& GetUpdatedDate() {
          UpdateDate();
          return date;
        }
        DirtyValue<TimeState> const& GetUpdatedTime() {
          UpdateTime();
          return time;
        }

        void UpdateBattery();
        DirtyValue<BatteryState> const& GetBattery() const {
          return battery;
        }
        DirtyValue<BatteryState> const& GetUpdatedBattery() {
          UpdateBattery();
          return battery;
        }

        void UpdateBle();
        DirtyValue<BleState> const& GetBle() const {
          return ble;
        }
        DirtyValue<BleState> const& GetUpdatedBle() {
          UpdateBle();
          return ble;
        }

        void UpdateNotifications();
        DirtyValue<NotificationState> const& GetNotifications() const {
          return notifications;
        }
        DirtyValue<NotificationState> const& GetUpdatedNotifications() {
          UpdateNotifications();
          return notifications;
        }

        void UpdateHeartRate();
        DirtyValue<HeartRateState> const& GetHeartRate() const {
          return heartRate;
        }
        DirtyValue<HeartRateState> const& GetUpdatedHeartRate() {
          UpdateHeartRate();
          return heartRate;
        }

      private:
        DirtyValue<DateState> date;
        DirtyValue<TimeState> time;
        DirtyValue<BatteryState> battery;
        DirtyValue<BleState> ble;
        DirtyValue<NotificationState> notifications;
        DirtyValue<HeartRateState> heartRate;

        Controllers::Settings& settingsController;
        Controllers::DateTimeController const& dateTimeController;
        Controllers::Battery const& batteryController;
        Controllers::Ble const& bleController;
        Controllers::NotificationManager const& notificationManager;
        Controllers::HeartRateController const& heartRateController;
      };
    }
  }
}

