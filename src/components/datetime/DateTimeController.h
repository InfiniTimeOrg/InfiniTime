#pragma once

#include <cstdint>
#include <chrono>
#include <string>
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    class DateTime {
    public:
      DateTime(Controllers::Settings& settingsController);
      enum class Days : uint8_t { Unknown, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday };
      enum class Months : uint8_t {
        Unknown,
        January,
        February,
        March,
        April,
        May,
        June,
        July,
        August,
        September,
        October,
        November,
        December
      };

      static const uint8_t NUMBER_OF_WORLD_TIMES = 4;

      void SetTime(uint16_t year,
                   uint8_t month,
                   uint8_t day,
                   uint8_t dayOfWeek,
                   uint8_t hour,
                   uint8_t minute,
                   uint8_t second,
                   uint32_t systickCounter);

      /*
       * setter corresponding to the BLE Set Local Time characteristic.
       *
       * used to update difference between utc and local time (see UtcOffset())
       *
       * parameters are in quarters of an our. Following the BLE CTS specification,
       * timezone is expected to be constant over DST which will be reported in
       * dst field.
       */
      // TODO test negative zones
      void SetTimeZone(uint8_t timezone, uint8_t dst);

      void updateWorldTime(uint8_t index, int8_t offset, const char* description);

      void UpdateTime(uint32_t systickCounter);
      uint16_t Year() const {
        return year;
      }
      Months Month() const {
        return month;
      }
      uint8_t Day() const {
        return day;
      }
      Days DayOfWeek() const {
        return dayOfWeek;
      }
      uint8_t Hours() const {
        return hour;
      }
      uint8_t Minutes() const {
        return minute;
      }
      uint8_t Seconds() const {
        return second;
      }

      /*
       * returns the offset between local time and UTC in quarters of an hour
       *
       * Availability of this field depends on wether the companion app
       * supports the BLE CTS Local Time Characteristic. Expect it to be 0
       * if not.
       */
      uint8_t UtcOffset() const {
        return tzOffset + dstOffset;
      }

      /*
       * returns the offset between the (dst independent) local time zone and UTC
       * in quarters of an hour
       *
       * Availability of this field depends on wether the companion app
       * supports the BLE CTS Local Time Characteristic. Expect it to be 0
       * if not.
       */
      uint8_t TzOffset() const {
        return tzOffset;
      }

      /*
       * returns the offset between the local time zone and local time
       * in quarters of an hour
       * if != 0, DST is in effect, if == 0 not.
       *
       * Availability of this field depends on wether the companion app
       * supports the BLE CTS Local Time Characteristic. Expect it to be 0
       * if not.
       */
      uint8_t DstOffset() const {
        return dstOffset;
      }

      int8_t worldOffset(uint8_t index) const;
      bool isWorldTimeEnabled(uint8_t index) const;
      const char* worldDescription(uint8_t index) const;

      const char* MonthShortToString() const;
      const char* DayOfWeekShortToString() const;
      static const char* MonthShortToStringLow(Months month);
      const char* DayOfWeekShortToStringLow() const;

      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> CurrentDateTime() const {
        return currentDateTime;
      }

      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> UTCDateTime() const {
        return currentDateTime - std::chrono::seconds((tzOffset + dstOffset) * 15 * 60);
      }

      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> WorldDateTime(uint8_t index) const {
        return UTCDateTime() + std::chrono::seconds(worldOffset(index) * 15 * 60);
      }

      std::chrono::seconds Uptime() const {
        return uptime;
      }

      void Register(System::SystemTask* systemTask);
      void SetCurrentTime(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> t);
      std::string FormattedTime();

    private:
      uint16_t year = 0;
      Months month = Months::Unknown;
      uint8_t day = 0;
      Days dayOfWeek = Days::Unknown;
      uint8_t hour = 0;
      uint8_t minute = 0;
      uint8_t second = 0;
      uint8_t tzOffset = 0;
      uint8_t dstOffset = 0;

      struct WorldTimeData {
        int8_t offset;
        char description[9];
      };

      WorldTimeData worldTimeData[NUMBER_OF_WORLD_TIMES];

      uint32_t previousSystickCounter = 0;
      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> currentDateTime;
      std::chrono::seconds uptime {0};

      bool isMidnightAlreadyNotified = false;
      bool isHourAlreadyNotified = true;
      bool isHalfHourAlreadyNotified = true;
      System::SystemTask* systemTask = nullptr;
      Controllers::Settings& settingsController;
    };
  }
}
