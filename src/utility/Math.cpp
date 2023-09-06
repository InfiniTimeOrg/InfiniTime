#include "utility/Math.h"

#include <lvgl/src/lv_misc/lv_math.h>

using namespace Pinetime::Utility;

#ifndef PINETIME_IS_RECOVERY

int16_t Pinetime::Utility::Asin(int16_t arg) {
  int16_t a = arg < 0 ? -arg : arg;

  int16_t angle = 45;
  int16_t low = 0;
  int16_t high = 90;
  while (low <= high) {
    int16_t sinAngle = _lv_trigo_sin(angle);
    int16_t sinAngleSub = _lv_trigo_sin(angle - 1);
    int16_t sinAngleAdd = _lv_trigo_sin(angle + 1);

    if (a >= sinAngleSub && a <= sinAngleAdd) {
      if (a <= (sinAngleSub + sinAngle) / 2) {
        angle--;
      } else if (a > (sinAngle + sinAngleAdd) / 2) {
        angle++;
      }
      break;
    }

    if (a < sinAngle) {
      high = angle - 1;
    }

    else {
      low = angle + 1;
    }

    angle = (low + high) / 2;
  }

  return arg < 0 ? -angle : angle;
}

#else

int16_t Pinetime::Utility::Asin(int16_t /*arg*/) {
  return 0;
}

#endif
