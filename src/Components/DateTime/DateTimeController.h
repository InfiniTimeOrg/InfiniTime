#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class DateTime {
      public:
        enum class Days : uint8_t {Unknown, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday};
        enum class Months : uint8_t {Unknown, January, February, March, April, May, June, July, August, September, October, November, December};

        void UpdateTime(uint16_t year, Months month, uint8_t day, Days dayOfWeek, uint8_t hour, uint8_t minute, uint8_t second);
        uint16_t Year() const { return year; }
        Months Month() const { return month; }
        uint8_t Day() const { return day; }
        Days DayOfWeek() const { return dayOfWeek; }
        uint8_t Hours() const { return hour; }
        uint8_t Minutes() const { return minute; }
        uint8_t Seconds() const { return second; }
      private:
        uint16_t year = 0;
        Months month = Months::Unknown;
        uint8_t day = 0;
        Days dayOfWeek = Days::Unknown;
        uint8_t hour = 0;
        uint8_t minute = 0;
        uint8_t second = 0;
    };
  }
}