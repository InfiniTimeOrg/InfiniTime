#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Utility {
    // returns the arcsin of `arg`. asin(-32767) = -90, asin(32767) = 90
    int16_t Asin(int16_t arg);
  }
}
