#include "displayapp/screens/settings/SettingClockFormat.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  struct ClockOption {
    Pinetime::Controllers::Settings::ClockType clockType;
    const char* name;
  };

  constexpr std::array<ClockOption, 2> clockOptions = {{
    {Pinetime::Controllers::Settings::ClockType::H12, "12-hour"},
    {Pinetime::Controllers::Settings::ClockType::H24, "24-hour"},
  }};

  std::array<CheckboxList::Item, CheckboxList::MaxItems> CreateClockOptionArray() {
    std::array<Pinetime::Applications::Screens::CheckboxList::Item, CheckboxList::MaxItems> clockOptionArray;
    for (size_t i = 0; i < CheckboxList::MaxItems; i++) {
      if (i >= clockOptions.size()) {
        clockOptionArray[i].name = "";
        clockOptionArray[i].enabled = false;
      } else {
        clockOptionArray[i].name = clockOptions[i].name;
        clockOptionArray[i].enabled = true;
      }
    }
    return clockOptionArray;
  }

  uint32_t GetDefaultClockOption(Pinetime::Controllers::Settings::ClockType currentOption) {
    for (size_t i = 0; i < clockOptions.size(); i++) {
      if (clockOptions[i].clockType == currentOption) {
        return i;
      }
    }
    return 0;
  }
}

SettingClockFormat::SettingClockFormat(Pinetime::Controllers::Settings& settingsController)
  : clockCheckboxList(
      0,
      1,
      "Time format",
      Symbols::clock,
      GetDefaultClockOption(settingsController.GetClockType()),
      [&settings = settingsController](uint32_t index) {
        settings.SetClockType(clockOptions[index].clockType);
        settings.SaveSettings();
      },
      CreateClockOptionArray()) {
}

SettingClockFormat::~SettingClockFormat() {
  lv_obj_clean(lv_scr_act());
}
