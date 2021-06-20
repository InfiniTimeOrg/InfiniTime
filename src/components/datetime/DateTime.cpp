#include "DateTime.h"

#include <array>

namespace Pinetime {
  namespace DateTime {
    namespace {

      constexpr std::array<char const*, 8> DaysStringLow = {
        "--",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
        "Sunday"
      };

      constexpr std::array<char const*, 8> DaysStringShortLow = {
        "--",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat",
        "Sun"
      };

      constexpr std::array<char const*, 8> DaysStringShort = {
        "--",
        "MON",
        "TUE",
        "WED",
        "THU",
        "FRI",
        "SAT",
        "SUN"
      };

      constexpr std::array<char const*, 8> DaysString = {
        "--",
        "MONDAY",
        "TUESDAY",
        "WEDNESDAY",
        "THURSDAY",
        "FRIDAY",
        "SATURDAY",
        "SUNDAY"
      };

      constexpr std::array<char const*, 13> MonthsString = {
        "--",
        "JAN",
        "FEB",
        "MAR",
        "APR",
        "MAY",
        "JUN",
        "JUL",
        "AUG",
        "SEP",
        "OCT",
        "NOV",
        "DEC"
      };

      constexpr std::array<char const*, 13> MonthsStringLow = {
        "--",
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
      };

      constexpr std::array<char const*, 13> MonthsLow = {
        "--",
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December"
      };

      template<typename E>
      constexpr typename std::underlying_type<E>::type to_underlying(E e) {
        return static_cast<typename std::underlying_type<E>::type>(e);
      }
    }

    char const* MonthShortToString(Months month) {
      return MonthsString[to_underlying(month)];
    }

    char const* MonthShortToStringLow(Months month) {
      return MonthsStringLow[to_underlying(month)];
    }

    char const* MonthsToStringLow(Months month) {
      return MonthsLow[to_underlying(month)];
    }

    char const* DayOfWeekToString(Days day) {
      return DaysString[to_underlying(day)];
    }

    char const* DayOfWeekShortToString(Days day) {
      return DaysStringShort[to_underlying(day)];
    }

    char const* DayOfWeekToStringLow(Days day) {
      return DaysStringLow[to_underlying(day)];
    }

    char const* DayOfWeekShortToStringLow(Days day) {
      return DaysStringShortLow[to_underlying(day)];
    }

  }
}
