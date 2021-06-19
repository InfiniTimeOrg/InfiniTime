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

std::unique_ptr<Screen> SettingWatchFace::CreateScreen() {
  options[settingsController.GetClockFace()].state = true;

  return std::make_unique<Screens::CheckBoxes>(Symbols::clock, "Watch face", options, app);
}

SettingWatchFace::~SettingWatchFace() {
  for (uint8_t i = 0; i < 2; i++) {
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
