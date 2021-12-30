#pragma once

#include <systemtask/SystemTask.h>
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Controllers {

    class ShowerController {
    public:
      ShowerController();

      void Register(System::SystemTask* systemTask);
      void SetSettingController(Pinetime::Controllers::Settings* settingsController);
      void ToggleShowerMode();
      bool IsShowerModeOn() const {
        return isShowerModeOn;
      }

    private:
      static ShowerController* instance;

      bool isShowerModeOn = false;
      uint32_t lastScreenTimeout = 0;
      Pinetime::Controllers::Settings::WakeUpMode lastWakeUpMode;

      Pinetime::Controllers::Settings* settingsController = nullptr;
      Pinetime::System::SystemTask* systemTask = nullptr;
    };
  }
}