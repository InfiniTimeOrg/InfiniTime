#include "SettingWatchFace.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

SettingWatchFace::SettingWatchFace(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController}, screen {[this, &settingsController]() {
      return CreateScreen();
    }()} {
}

bool SettingWatchFace::UpdateArray(Pinetime::Applications::Screens::CheckBoxes::Options* options, uint8_t clicked) {
  return false;
}

std::unique_ptr<Screen> SettingWatchFace::CreateScreen() {
  options[settingsController.GetClockFace()].state = true;

  return std::make_unique<Screens::CheckBoxes>(Symbols::home, "Watch face", options, app, UpdateArray);
}

SettingWatchFace::~SettingWatchFace() {
  for (uint8_t i = 0; i < 3; i++) {
    if (options[i].state == true) {
      settingsController.SetClockFace(i);
      break;
    }
  }

  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

bool SettingWatchFace::Refresh() {
  return screen->Refresh();
}
