#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    static constexpr uint8_t pinMotor = 16;

    class MotorController {
      public:
        void Init();
        void SetDuration(uint8_t motorDuration);

        APP_TIMER_DEF(vibTimer);

      private:

    };
  }
}