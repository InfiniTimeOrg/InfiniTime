#include "displayapp/screens/settings/SettingWeatherFormat.h"

#include <lvgl/lvgl.h>

#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    Pinetime::Controllers::Settings::WeatherFormat weatherFormat;
    const char* name;
  };

  constexpr std::array<Option, 2> options = {{
    {Pinetime::Controllers::Settings::WeatherFormat::Metric, "Metric"},
    {Pinetime::Controllers::Settings::WeatherFormat::Imperial, "Imperial"},
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

  uint32_t GetDefaultOption(Pinetime::Controllers::Settings::WeatherFormat currentOption) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].weatherFormat == currentOption) {
        return i;
      }
    }
    return 0;
  }
}

SettingWeatherFormat::SettingWeatherFormat(Pinetime::Controllers::Settings& settingsController)
  : checkboxList(
      0,
      1,
      "Weather format",
      Symbols::clock,
      GetDefaultOption(settingsController.GetWeatherFormat()),
      [&settings = settingsController](uint32_t index) {
        settings.SetWeatherFormat(options[index].weatherFormat);
        settings.SaveSettings();
      },
      CreateOptionArray()) {
}

SettingWeatherFormat::~SettingWeatherFormat() {
  lv_obj_clean(lv_scr_act());
}
