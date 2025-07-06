#pragma once

#include <cstdint>
#include <concepts>

namespace Pinetime {
  namespace Utility {
    // returns the arcsin of `arg`. asin(-32767) = -90, asin(32767) = 90
    int16_t Asin(int16_t arg);

    static constexpr auto RoundedDiv(std::integral auto dividend, std::unsigned_integral auto divisor) -> decltype(dividend / divisor) {
      return (dividend + (dividend >= 0 ? divisor : -divisor) / 2) / divisor;
    }
  }
}
