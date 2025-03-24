#include "displayapp/screens/settings/Settings.h"
#include <lvgl/lvgl.h>
#include <functional>
#include "displayapp/apps/Apps.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

constexpr std::array<List::Applications, Settings::entries.size()> Settings::entries;

auto Settings::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screens;
  for (size_t i = 0; i < screens.size(); i++) {
    screens[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screens;
}

Settings::Settings(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app},
    settingsController {settingsController},
    screens {app, settingsController.GetSettingsMenu(), CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

Settings::~Settings() {
  lv_obj_clean(lv_scr_act());
}

bool Settings::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> Settings::CreateScreen(unsigned int screenNum) const {
  std::array<List::Applications, entriesPerScreen> screens;
  for (int i = 0; i < entriesPerScreen; i++) {
    screens[i] = entries[screenNum * entriesPerScreen + i];
  }

  return std::make_unique<Screens::List>(screenNum, nScreens, app, settingsController, screens);
}
