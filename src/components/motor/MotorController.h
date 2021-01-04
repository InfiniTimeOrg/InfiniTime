#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class MotorController {
    public:
      void Init();
      void SetDuration(uint8_t motorDuration);

    private:
      static constexpr uint8_t pinMotor = 16;
    };
  }
}
