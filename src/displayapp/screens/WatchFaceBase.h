#pragma once

#include "Screen.h"
#include "components/datetime/DateTime.h"
#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class DateTimeController;
    class Battery;
    class Ble;
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

        WatchFaceBase(DisplayApp* app,
                      Controllers::DateTimeController const& dateTimeController,
                      Controllers::Battery const& batteryController,
                      Controllers::Ble const& bleController);

        virtual ~WatchFaceBase() = default;

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

      private:
        DirtyValue<DateState> date;
        DirtyValue<TimeState> time;
        DirtyValue<BatteryState> battery;
        DirtyValue<BleState> ble;

        Controllers::DateTimeController const& dateTimeController;
        Controllers::Battery const& batteryController;
        Controllers::Ble const& bleController;
      };
    }
  }
}

