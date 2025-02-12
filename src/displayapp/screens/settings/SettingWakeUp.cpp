#include "displayapp/screens/settings/SettingWakeUp.h"
#include <lvgl/lvgl.h>

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/CheckboxList.h"
#include "displayapp/screens/Styles.h"

using namespace Pinetime::Applications::Screens;

constexpr const char* SettingWakeUp::title;
constexpr const char* SettingWakeUp::symbol;

constexpr std::array<SettingWakeUp::Option, SettingWakeUp::optionsCount> SettingWakeUp::options;

std::unique_ptr<Screen> SettingWakeUp::CreateScreen(unsigned int screenNum) const {
  std::array<Screens::CheckboxList::Item, settingsPerScreen> optionsOnThisScreen;
  for (int i = 0; i < settingsPerScreen; i++) {
    if (i + (screenNum * settingsPerScreen) >= options.size()) {
      optionsOnThisScreen[i] = {"", false};
    } else {
      auto& item = options[i + (screenNum * settingsPerScreen)];
      optionsOnThisScreen[i] = Screens::CheckboxList::Item {item.name, true};
    }
  }

  return std::make_unique<Screens::CheckboxList>(
    screenNum,
    nScreens,
    title,
    symbol,
    optionsCount,
    [this, &settings = settingsController](uint32_t index) {
      bool currentState = settingsController.isWakeUpModeOn(options[index].wakeUpMode);
      settingsController.setWakeUpMode(options[index].wakeUpMode, !currentState);
    },
    optionsOnThisScreen,
    false,
    [this, &settings = settingsController](uint32_t index) {
      bool currentState = settingsController.isWakeUpModeOn(options[index].wakeUpMode);
      return currentState;
    });
}

auto SettingWakeUp::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screens;
  for (size_t i = 0; i < screens.size(); i++) {
    screens[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screens;
}

SettingWakeUp::SettingWakeUp(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : settingsController {settingsController}, screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingWakeUp::~SettingWakeUp() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

bool SettingWakeUp::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}
