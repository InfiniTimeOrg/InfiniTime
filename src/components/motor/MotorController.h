#pragma once

#include <cstdint>
#include "app_timer.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"

namespace Pinetime {
  namespace Controllers {
    static constexpr uint8_t pinMotor = 16;

    class MotorController {
    public:
      MotorController(Controllers::Settings& settingsController, Controllers::Battery& batteryController);
      void Init();
      void SetDuration(uint8_t motorDuration);

    private:
      Controllers::Battery& batteryController;
      Controllers::Settings& settingsController;
      static void vibrate(void* p_context);
    };
  }
}
