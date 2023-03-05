#include "displayapp/screens/ApplicationList.h"
#include <lvgl/lvgl.h>
#include <functional>
#include "displayapp/Apps.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

constexpr std::array<Tile::Applications, ApplicationList::applications.size()> ApplicationList::applications;

auto ApplicationList::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screens;
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
                                 Controllers::DateTime& dateTimeController)
  : app {app},
    settingsController {settingsController},
    batteryController {batteryController},
    bleController {bleController},
    dateTimeController {dateTimeController},
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
    apps[i] = applications[screenNum * appsPerScreen + i];
  }

  return std::make_unique<Screens::Tile>(screenNum,
                                         nScreens,
                                         app,
                                         settingsController,
                                         batteryController,
                                         bleController,
                                         dateTimeController,
                                         apps);
}
