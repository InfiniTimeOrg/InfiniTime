#include "SettingWakeUp.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

SettingWakeUp::SettingWakeUp(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController}, screen {[this, &settingsController]() {
      return CreateScreen();
    }()} {
}

std::unique_ptr<Screen> SettingWakeUp::CreateScreen() {
  switch (settingsController.getWakeUpMode()) {
    case Controllers::Settings::WakeUpMode::None:
      options[0].state = true;
      break;
    case Controllers::Settings::WakeUpMode::SingleTap:
      options[1].state = true;
      break;
    case Controllers::Settings::WakeUpMode::DoubleTap:
      options[2].state = true;
      break;
    case Controllers::Settings::WakeUpMode::RaiseWrist:
      options[3].state = true;
      break;
  }

  return std::make_unique<Screens::CheckBoxes>(Symbols::clock, "Wake up", options, app);
}

SettingWakeUp::~SettingWakeUp() {
  if (options[0].state == true) {
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::None);
  } else if (options[1].state == true) {
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::SingleTap);
  } else if (options[2].state == true) {
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap);
  } else if (options[3].state == true) {
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::RaiseWrist);
  }

  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

bool SettingWakeUp::Refresh() {
  return screen->Refresh();
}
