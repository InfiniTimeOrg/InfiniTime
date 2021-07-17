#include "SettingWakeUp.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

bool SettingWakeUp::UpdateArray(Pinetime::Applications::Screens::CheckBoxes::Options* options, uint8_t clicked) {
  options[clicked].state = !options[clicked].state;
  if (clicked == 0) {
    if (options[0].state) {
      options[1].state = false;
    }
  } else if (clicked == 1) {
    if (options[1].state) {
      options[0].state = false;
    }
  }
  return true;
}

SettingWakeUp::SettingWakeUp(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController}, screen {[this, &settingsController]() {
      return CreateScreen();
    }()} {
}

std::unique_ptr<Pinetime::Applications::Screens::CheckBoxes> SettingWakeUp::CreateScreen() {
  if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::SingleTap)) {
    options[0].state = true;
  }
  if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
    options[1].state = true;
  }
  if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::RaiseWrist)) {
    options[2].state = true;
  }

  return std::make_unique<Screens::CheckBoxes>(Symbols::sun, "Wake Up", options, app, UpdateArray);
}

SettingWakeUp::~SettingWakeUp() {
  settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::SingleTap, options[0].state);
  settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap, options[1].state);
  settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::RaiseWrist, options[2].state);

  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

bool SettingWakeUp::Refresh() {
  return screen->Refresh();
}
