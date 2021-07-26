#include "SettingDisplay.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/Messages.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

SettingDisplay::SettingDisplay(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController}, screen {[this, &settingsController]() {
      return CreateScreen();
    }()} {
}

bool SettingDisplay::UpdateArray(Pinetime::Applications::Screens::CheckBoxes::Options* options, uint8_t clicked) {
  return false;
}

std::unique_ptr<Screen> SettingDisplay::CreateScreen() {
  switch (settingsController.GetScreenTimeOut()) {
    case 5000:
      options[0].state = true;
      break;
    case 10000:
      options[1].state = true;
      break;
    case 15000:
      options[2].state = true;
      break;
    case 20000:
      options[3].state = true;
      break;
    case 25000:
      options[4].state = true;
      break;
    case 30000:
      options[5].state = true;
      break;
  }

  return std::make_unique<Screens::CheckBoxes>(Symbols::sun, "Display timeout", options, app, UpdateArray);
}

SettingDisplay::~SettingDisplay() {
  for (uint8_t i = 0; i < 6; i++) {
    if (options[i].state == true) {
      settingsController.SetScreenTimeOut((i + 1) * 5000);
      break;
    }
  }
  app->PushMessage(Applications::Display::Messages::UpdateTimeOut);
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

bool SettingDisplay::Refresh() {
  return screen->Refresh();
}
