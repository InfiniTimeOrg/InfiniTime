#include "displayapp/screens/settings/SettingTimeFormat.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    Pinetime::Controllers::Settings::ClockType clockType;
    const char* name;
  };

  constexpr std::array<Option, 2> options = {{
    {Pinetime::Controllers::Settings::ClockType::H12, "12-hour"},
    {Pinetime::Controllers::Settings::ClockType::H24, "24-hour"},
  }};

  std::array<CheckboxList::Item, CheckboxList::MaxItems> CreateOptionArray() {
    std::array<Pinetime::Applications::Screens::CheckboxList::Item, CheckboxList::MaxItems> optionArray;
    for (size_t i = 0; i < CheckboxList::MaxItems; i++) {
      if (i >= options.size()) {
        optionArray[i].name = "";
        optionArray[i].enabled = false;
      } else {
        optionArray[i].name = options[i].name;
        optionArray[i].enabled = true;
      }
    }
    return optionArray;
  }

  uint32_t GetDefaultOption(Pinetime::Controllers::Settings::ClockType currentOption) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].clockType == currentOption) {
        return i;
      }
    }
    return 0;
  }
}

SettingTimeFormat::SettingTimeFormat(Pinetime::Controllers::Settings& settingsController)
  : checkboxList(
      0,
      1,
      "Time format",
      Symbols::clock,
      GetDefaultOption(settingsController.GetClockType()),
      [&settings = settingsController](uint32_t index) {
        settings.SetClockType(options[index].clockType);
        settings.SaveSettings();
      },
      CreateOptionArray()) {
}

SettingTimeFormat::~SettingTimeFormat() {
  lv_obj_clean(lv_scr_act());
}
