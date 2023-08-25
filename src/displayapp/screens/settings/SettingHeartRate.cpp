#include "displayapp/screens/settings/SettingHeartRate.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <array>
#include <algorithm>

using namespace Pinetime::Applications::Screens;

constexpr const char* SettingHeartRate::title;
constexpr const char* SettingHeartRate::symbol;

namespace {
  constexpr std::array<uint32_t, 6> intervalOptions = {{
    0,
    30 * 1000,
    60 * 1000,
    5 * 60 * 1000,
    10 * 60 * 1000,
    30 * 60 * 1000,
  }};

  constexpr std::array<CheckboxList::Item, 8> options = {{
    {"Off", true},
    {"30s", true},
    {"1 min", true},
    {"5 min", true},
    {"10 min", true},
    {"30 min", true},
    {"", false},
    {"", false},
  }};

  uint32_t GetDefaultOption(uint32_t currentInterval) {
    for (size_t i = 0; i < intervalOptions.size(); i++) {
      if (intervalOptions[i] == currentInterval) {
        return i;
      }
    }
    return 0;
  }
}

auto SettingHeartRate::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screens;
  for (size_t i = 0; i < screens.size(); i++) {
    screens[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screens;
}

SettingHeartRate::SettingHeartRate(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settings)
  : app {app}, settings {settings}, screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingHeartRate::~SettingHeartRate() {
  lv_obj_clean(lv_scr_act());
}

bool SettingHeartRate::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SettingHeartRate::CreateScreen(unsigned int screenNum) const {
  std::array<Screens::CheckboxList::Item, optionsPerScreen> optionsOnThisScreen;
  for (int i = 0; i < optionsPerScreen; i++) {
    optionsOnThisScreen[i] = options[screenNum * optionsPerScreen + i];
  }

  return std::make_unique<Screens::CheckboxList>(
    screenNum,
    nScreens,
    title,
    symbol,
    GetDefaultOption(settings.GetHeartRateBackgroundMeasurementInterval()),
    [&settings = settings](uint32_t index) {
      settings.SetHeartRateBackgroundMeasurementInterval(intervalOptions[index]);
      settings.SaveSettings();
    },
    optionsOnThisScreen);
}