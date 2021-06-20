#pragma once

#include <cstdint>

namespace Pinetime {
  namespace DateTime {
    enum class Days : uint8_t {
      Unknown,
      Monday,
      Tuesday,
      Wednesday,
      Thursday,
      Friday,
      Saturday,
      Sunday
    };

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

    char const* MonthShortToString(Months month);
    char const* MonthShortToStringLow(Months month);
    char const* MonthsToStringLow(Months month);
    char const* DayOfWeekToString(Days day);
    char const* DayOfWeekShortToString(Days day);
    char const* DayOfWeekToStringLow(Days day);
    char const* DayOfWeekShortToStringLow(Days day);
  }
}
