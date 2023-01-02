#include "displayapp/screens/settings/SettingTimeFormat.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

constexpr std::array<SettingTimeFormat::Option, 2> SettingTimeFormat::options;

SettingTimeFormat::SettingTimeFormat(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)

  : Screen(app),
    settingsController {settingsController},
    settingScreen("Time format", Symbols::clock, entries, nEntries, objectArray, true) {
}

SettingTimeFormat::~SettingTimeFormat() {
  lv_obj_clean(lv_scr_act());
  for (size_t i = 0; i < nEntries; i++) {
    if (lv_checkbox_is_checked(objectArray[i])) {
      settingsController.SetClockType(options[i].clockType);
    }
  }
  settingsController.SaveSettings();
}
