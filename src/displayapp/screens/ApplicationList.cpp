#include <lvgl/lvgl.h>
#include <functional>
#include <vector>

#include "displayapp/screens/ApplicationList.h"
#include "displayapp/Apps.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

// constexpr std::array<Tile::Applications, ApplicationList::applications.size()> ApplicationList::applications;

auto ApplicationList::CreateScreenList() const {
  const uint8_t nApps = appController.NApps();
  const uint8_t nScreens = nApps / appsPerScreen + !!(nApps % appsPerScreen);

  std::vector<std::function<std::unique_ptr<Screen>()>> screens;
  screens.resize(nScreens);

  for (size_t i = 0; i < screens.size(); i++) {
    screens[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screens;
}

ApplicationList::ApplicationList(Pinetime::Applications::DisplayApp* app,
                                 Pinetime::Controllers::Settings& settingsController,
                                 const Pinetime::Controllers::Battery& batteryController,
                                 const Pinetime::Controllers::Ble& bleController,
                                 Controllers::DateTime& dateTimeController,
                                 Pinetime::Applications::AppController& appController)
  : app {app},
    settingsController {settingsController},
    batteryController {batteryController},
    bleController {bleController},
    dateTimeController {dateTimeController},
    appController {appController},
    screens {app, settingsController.GetAppMenu(), CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

ApplicationList::~ApplicationList() {
  lv_obj_clean(lv_scr_act());
}

bool ApplicationList::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> ApplicationList::CreateScreen(unsigned int screenNum) const {
  std::array<Tile::Applications, appsPerScreen> apps;
  for (int i = 0; i < appsPerScreen; i++) {
    if (i >= appController.NApps()) {
      apps[i] = {nullptr, static_cast<uint8_t>(Apps::None)};
    } else {
      uint8_t appId = screenNum * appsPerScreen + i + static_cast<uint8_t>(Apps::Dynamic);
      apps[i] = {appController.GetSymbol(appId), appId};
    }
  }

  const uint8_t nApps = appController.NApps();
  const uint8_t nScreens = nApps / appsPerScreen + !!(nApps % appsPerScreen);

  return std::make_unique<Screens::Tile>(screenNum,
                                         nScreens,
                                         app,
                                         settingsController,
                                         batteryController,
                                         bleController,
                                         dateTimeController,
                                         apps);
}
