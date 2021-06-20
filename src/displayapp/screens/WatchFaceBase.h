#pragma once

#include "Screen.h"
#include "../DisplayApp.h"
#include "components/datetime/DateTime.h"
#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class DateTimeController;
  }
  namespace Applications {
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

        WatchFaceBase(DisplayApp* app,
                      Controllers::DateTimeController const& dateTimeController);

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

      private:
        DirtyValue<DateState> date;
        DirtyValue<TimeState> time;

        Controllers::DateTimeController const& dateTimeController;
      };
    }
  }
}

