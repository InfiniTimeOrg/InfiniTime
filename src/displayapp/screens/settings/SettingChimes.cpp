#include "displayapp/screens/settings/SettingChimes.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/localization/Localization.h"
#include <array>

using namespace Pinetime::Applications::Screens;
using namespace Pinetime::Applications::Localization;

namespace {
  struct Option {
    Pinetime::Controllers::Settings::ChimesOption chimesOption;
    StringId name;
  };

  constexpr std::array<Option, 3> options = {{
    {Pinetime::Controllers::Settings::ChimesOption::None, StringId::Off},
    {Pinetime::Controllers::Settings::ChimesOption::Hours, StringId::EveryHour},
    {Pinetime::Controllers::Settings::ChimesOption::HalfHours, StringId::Every30Minutes},
  }};

  std::array<CheckboxList::Item, CheckboxList::MaxItems> CreateOptionArray(Language language) {
    std::array<Pinetime::Applications::Screens::CheckboxList::Item, CheckboxList::MaxItems> optionArray;
    for (size_t i = 0; i < CheckboxList::MaxItems; i++) {
      if (i >= options.size()) {
        optionArray[i].name = "";
        optionArray[i].enabled = false;
      } else {
        optionArray[i].name = Translate(language, options[i].name);
        optionArray[i].enabled = true;
      }
    }
    return optionArray;
  }

  uint32_t GetDefaultOption(Pinetime::Controllers::Settings::ChimesOption currentOption) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].chimesOption == currentOption) {
        return i;
      }
    }
    return 0;
  }
}

SettingChimes::SettingChimes(Pinetime::Controllers::Settings& settingsController)
  : checkboxList(
      0,
      1,
      Translate(settingsController.GetLanguage(), StringId::Chimes),
      Symbols::clock,
      GetDefaultOption(settingsController.GetChimeOption()),
      [&settings = settingsController](uint32_t index) {
        settings.SetChimeOption(options[index].chimesOption);
        settings.SaveSettings();
      },
      CreateOptionArray(settingsController.GetLanguage())) {
}

SettingChimes::~SettingChimes() {
  lv_obj_clean(lv_scr_act());
}
