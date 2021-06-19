#include "SettingTimeFormat.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

SettingTimeFormat::SettingTimeFormat(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController}, screen {[this, &settingsController]() {
      return CreateScreen();
    }()} {
}

std::unique_ptr<Screen> SettingTimeFormat::CreateScreen() {
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    options[0].state = true;
  } else {
    options[1].state = true;
  }

  return std::make_unique<Screens::CheckBoxes>(Symbols::clock, "Time format", options, app);
}

SettingTimeFormat::~SettingTimeFormat() {
  if (options[0].state == true) {
    settingsController.SetClockType(Controllers::Settings::ClockType::H12);
  } else {
    settingsController.SetClockType(Controllers::Settings::ClockType::H24);
  }
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

bool SettingTimeFormat::Refresh() {
  return screen->Refresh();
}
