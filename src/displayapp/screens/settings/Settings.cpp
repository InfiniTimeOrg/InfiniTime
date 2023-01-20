#include "displayapp/screens/settings/Settings.h"
#include <lvgl/lvgl.h>
#include <functional>
#include "displayapp/Apps.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

constexpr List::Applications Settings::entries[];

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
  : Screen(app),
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
  static constexpr int entriesPerScreen = List::maxListItems;

  std::array<List::Applications, entriesPerScreen> screens = {Symbols::none, "", Apps::None};
  for (int i = 0; i < entriesPerScreen; i++) {
    size_t appIndex = screenNum * entriesPerScreen + i;
    if (appIndex > nEntries - 1) {
      break;
    }
    screens[i] = entries[appIndex];
  }

  return std::make_unique<Screens::List>(screenNum, nScreens, app, settingsController, screens);
}
