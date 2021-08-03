#pragma once

#include <cstdint>
#include "app_timer.h"
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Controllers {

    class MotorController {
    public:
      MotorController(Controllers::Settings& settingsController);
      void Init();
      void SetDuration(uint8_t motorDuration);

    private:
      Controllers::Settings& settingsController;
      static void vibrate(void* p_context);
    };
  }
}
