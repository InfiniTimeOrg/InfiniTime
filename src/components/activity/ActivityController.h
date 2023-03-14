#pragma once

#include <array>
#include <cstdint>

#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "components/motion/MotionController.h"
#include "utility/CircularBuffer.h"

namespace Pinetime {
  namespace Controllers {
    class ActivityController {
    public:
      void UpdateSteps(uint32_t nbSteps, uint8_t minutes);
      bool ShouldNotify(uint16_t thresh);

    private:
      Utility::CircularBuffer<uint32_t, 4> stepHistory = {};
      uint8_t prevHourSegment = UINT8_MAX;
      bool canNotify = false;
    };
  }
}
