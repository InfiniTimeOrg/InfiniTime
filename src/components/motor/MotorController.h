#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Controllers {

    class MotorController {
    public:
      MotorController() = default;

      void Init();
      void RunForDuration(uint8_t motorDuration);
      void StartRinging();
      static void StopRinging();

    private:
      static void Ring(void* p_context);
      static void StopMotor(void* p_context);
    };
  }
}
