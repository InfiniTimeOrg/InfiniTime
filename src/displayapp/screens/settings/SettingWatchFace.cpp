#include "displayapp/screens/settings/SettingWatchFace.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

constexpr const char* SettingWatchFace::title;
constexpr const char* SettingWatchFace::symbol;

auto SettingWatchFace::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screens;
  for (size_t i = 0; i < screens.size(); i++) {
    screens[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screens;
}

SettingWatchFace::SettingWatchFace(Pinetime::Applications::DisplayApp* app,
                                   Pinetime::Controllers::Settings& settingsController,
                                   Pinetime::Controllers::FS& filesystem)
  : app {app},
    settingsController {settingsController},
    filesystem {filesystem},
    screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingWatchFace::~SettingWatchFace() {
  lv_obj_clean(lv_scr_act());
}

bool SettingWatchFace::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SettingWatchFace::CreateScreen(unsigned int screenNum) const {
  std::array<Screens::CheckboxList::Item, settingsPerScreen> watchfacesOnThisScreen;
  for (int i = 0; i < settingsPerScreen; i++) {
    watchfacesOnThisScreen[i] = watchfaces[screenNum * settingsPerScreen + i];
  }

  return std::make_unique<Screens::CheckboxList>(
    screenNum,
    nScreens,
    title,
    symbol,
    settingsController.GetClockFace(),
    [&settings = settingsController](uint32_t clockFace) {
      settings.SetClockFace(clockFace);
      settings.SaveSettings();
    },
    watchfacesOnThisScreen);
}
