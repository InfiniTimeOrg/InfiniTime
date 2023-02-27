#pragma once
#include "components/datetime/DateTimeController.h"
#include "components/motor/MotorController.h"
#include <FreeRTOS.h>
#include <timers.h>

namespace Pinetime {
  namespace Controllers {
    class PeriodicAlertController {
    public:
      PeriodicAlertController(Controllers::DateTime& dateTimeController,
                              Controllers::Settings& settingsController,
                              Controllers::MotorController& motorController);
      void Init(System::SystemTask* systemTask);
      void SetPeriodicAlertMinute(uint8_t minute);
      void Enable();
      void Disable();

      void Alert();
      void SetTimer();

    private:

      Controllers::Settings& settingsController;
      Controllers::DateTime& dateTimeController;
      Controllers::MotorController& motorController;
      uint8_t minute = 0;
      TimerHandle_t alertTimer;
    };
  }
}
