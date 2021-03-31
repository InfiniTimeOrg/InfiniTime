#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class MotionController {
    public:
      void Update(int16_t x, int16_t y, int16_t z, uint32_t nbSteps);

      uint16_t X() const { return x; }
      uint16_t Y() const { return y; }
      uint16_t Z() const { return z; }
      uint32_t NbSteps() const { return nbSteps; }
      bool ShouldWakeUp(bool isSleeping);

      private:
      uint32_t nbSteps;
      int16_t x;
      int16_t y;
      int16_t z;
      int16_t lastYForWakeUp = 0;

    };
  }
}