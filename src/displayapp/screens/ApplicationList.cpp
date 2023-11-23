#include "displayapp/screens/ApplicationList.h"
#include "displayapp/screens/Tile.h"
#include <lvgl/lvgl.h>
#include <functional>
#include <algorithm>
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

auto ApplicationList::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screens;
  for (size_t i = 0; i < screens.size(); i++) {
    screens[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screens;
}

ApplicationList::ApplicationList(DisplayApp* app,
                                 Pinetime::Controllers::Settings& settingsController,
                                 const Pinetime::Controllers::Battery& batteryController,
                                 const Pinetime::Controllers::Ble& bleController,
                                 Controllers::DateTime& dateTimeController,
                                 Pinetime::Controllers::FS& filesystem,
                                 std::array<Tile::Applications, UserAppTypes::Count>&& apps)
  : app {app},
    settingsController {settingsController},
    batteryController {batteryController},
    bleController {bleController},
    dateTimeController {dateTimeController},
    filesystem {filesystem},
    apps {std::move(apps)},
    screens {app, settingsController.GetAppMenu(), CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

ApplicationList::~ApplicationList() {
  lv_obj_clean(lv_scr_act());
}

bool ApplicationList::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> ApplicationList::CreateScreen(unsigned int screenNum) const {
  std::array<Tile::Applications, appsPerScreen> pageApps;

  for (int i = 0; i < appsPerScreen; i++) {
    if (i + (screenNum * appsPerScreen) >= apps.size()) {
      pageApps[i] = {"", Pinetime::Applications::Apps::None, false};
    } else {
      pageApps[i] = apps[i + (screenNum * appsPerScreen)];
    }
  }

  return std::make_unique<Screens::Tile>(screenNum,
                                         nScreens,
                                         app,
                                         settingsController,
                                         batteryController,
                                         bleController,
                                         dateTimeController,
                                         pageApps);
}
