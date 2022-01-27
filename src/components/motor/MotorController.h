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
      void StartRingingCoupleTimes(int8_t times);
      static void StopRinging();

    private:
      bool shouldStopRinging();
      
      static void Ring(void* p_context);
      static void StopMotor(void* p_context);

      int8_t timesToRing = 0;
    };
  }
}
