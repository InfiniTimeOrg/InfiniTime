#pragma once

#include <cstdint>
#include "app_timer.h"
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Controllers {
    static constexpr uint8_t pinMotor = 16;

    class MotorController {
    public:
      MotorController(Controllers::Settings& settingsController);
      void Init();
      void RunForDuration(uint8_t motorDuration);
      void startRunning(uint8_t motorDuration);
      void stopRunning();

    private:
      Controllers::Settings& settingsController;
      static void vibrate(void* p_context);
      bool isBusy;
      };
  }
}
