#include "components/shower/ShowerController.h"
#include "systemtask/Messages.h"

using namespace Pinetime::Controllers;

ShowerController* ShowerController::instance = nullptr;

ShowerController::ShowerController() {
  instance = this;
}

void ShowerController::ToggleShowerMode() {
  using WakeOn = Controllers::Settings::WakeUpMode;

  this->isShowerModeOn = !this->isShowerModeOn;
    bool settingsChanged = false;
    if (this->IsShowerModeOn()) {
      if (lastScreenTimeout != settingsController->GetScreenTimeOut()) {
        lastScreenTimeout = settingsController->GetScreenTimeOut();
        settingsController->SetScreenTimeOut(5000);
        settingsChanged = true;
      }
      if (settingsController->isWakeUpModeOn(WakeOn::SingleTap)) {
        lastWakeUpMode = WakeOn::SingleTap;
        settingsController->setWakeUpMode(WakeOn::SingleTap, false);
        settingsChanged = true;
      }
      else if (settingsController->isWakeUpModeOn(WakeOn::DoubleTap)) {
        lastWakeUpMode = WakeOn::DoubleTap;
        settingsController->setWakeUpMode(WakeOn::DoubleTap, false);
        settingsChanged = true;
      }
      else if (settingsController->isWakeUpModeOn(WakeOn::RaiseWrist)) {
        lastWakeUpMode = WakeOn::RaiseWrist;
        settingsController->setWakeUpMode(WakeOn::RaiseWrist, false);
        settingsChanged = true;
      }
      if (settingsChanged) {
        settingsController->SaveSettings();
      }
    } else {
      settingsController->SetScreenTimeOut(lastScreenTimeout);
      settingsController->setWakeUpMode(lastWakeUpMode, true);
      settingsController->SaveSettings();

    }
}

void ShowerController::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}

void ShowerController::SetSettingController(Pinetime::Controllers::Settings* settingsController) {
  this->settingsController = settingsController;
}