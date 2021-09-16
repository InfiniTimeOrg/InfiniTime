#pragma once

#include <cstdint>
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Controllers {

    class MotorController {
    public:
      MotorController(Controllers::Settings& settingsController);
      void Init();
      void RunForDuration(uint8_t motorDuration);
      void StartRinging();
      static void StopRinging();

    private:
      static void Ring(void* p_context);
      Controllers::Settings& settingsController;
      static void StopMotor(void* p_context);
    };
  }
}
