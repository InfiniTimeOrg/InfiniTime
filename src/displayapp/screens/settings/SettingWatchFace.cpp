#include "displayapp/screens/settings/SettingWatchFace.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

constexpr const char* SettingWatchFace::title;
constexpr const char* SettingWatchFace::symbol;

namespace {
  uint32_t IndexOf(const std::array<Pinetime::Applications::Screens::SettingWatchFace::Item,
                                    Pinetime::Applications::UserWatchFaceTypes::Count>& watchfaces,
                   Pinetime::Applications::WatchFace watchface) {
    size_t index = 0;
    auto found = std::find_if(watchfaces.begin(),
                              watchfaces.end(),
                              [&index, &watchface](const Pinetime::Applications::Screens::SettingWatchFace::Item& item) {
                                const bool result = item.watchface == watchface;
                                if (!result) {
                                  index++;
                                }
                                return result;
                              });
    if (found == watchfaces.end()) {
      index = 0;
    }

    return index;
  }

  Pinetime::Applications::WatchFace IndexToWatchFace(const std::array<Pinetime::Applications::Screens::SettingWatchFace::Item,
                                                                      Pinetime::Applications::UserWatchFaceTypes::Count>& watchfaces,
                                                     size_t index) {
    if (index >= watchfaces.size()) {
      return watchfaces[0].watchface;
    }
    return watchfaces[index].watchface;
  }
}

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
                                   std::array<Screens::SettingWatchFace::Item, UserWatchFaceTypes::Count>&& watchfaceItems,
                                   Pinetime::Controllers::Settings& settingsController,
                                   Pinetime::Controllers::FS& filesystem)
  : app {app},
    watchfaceItems {std::move(watchfaceItems)},
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
    if (i + (screenNum * settingsPerScreen) >= watchfaceItems.size()) {
      watchfacesOnThisScreen[i] = {"", false};
    } else {
      auto& item = watchfaceItems[i + (screenNum * settingsPerScreen)];
      watchfacesOnThisScreen[i] = Screens::CheckboxList::Item {item.name, item.enabled};
    }
  }

  return std::make_unique<Screens::CheckboxList>(
    screenNum,
    nScreens,
    title,
    symbol,
    static_cast<uint32_t>(IndexOf(watchfaceItems, settingsController.GetWatchFace())),
    [this, &settings = settingsController](uint32_t index) {
      settings.SetWatchFace(IndexToWatchFace(watchfaceItems, index));
      settings.SaveSettings();
    },
    watchfacesOnThisScreen);
}
