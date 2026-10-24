#pragma once

#include <cstdint>
#include <concepts>

namespace Pinetime {
  namespace Utility {
    // returns the arcsin of `arg`. asin(-32767) = -90, asin(32767) = 90
    int16_t Asin(int16_t arg);

    // Round half away from zero integer division
    // If T signed, divisor cannot be std::numeric_limits<T>::min()
    // Adapted from https://github.com/lucianpls/rounding_integer_division
    // Under the MIT license
    template <std::integral T>
    constexpr T RoundedDiv(T dividend, T divisor) {
      bool neg = divisor < 0;
      if (neg) {
        // overflows if divisor is minimum value for T
        divisor = -divisor;
      }

      T m = dividend % divisor;
      T h = divisor / 2 + divisor % 2;
      T res = (dividend / divisor) + (!(dividend < 0) & (m >= h)) - ((dividend < 0) & ((m + h) <= 0));

      if (neg) {
        res = -res;
      }
      return res;
    }
  }
}
