#include "displayapp/screens/settings/SettingDoubleClick.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    Pinetime::Controllers::Settings::DoubleClickAction action;
    const char* name;
  };

  constexpr std::array<Option, 2> options = {{
    {Pinetime::Controllers::Settings::DoubleClickAction::GoToNotifications, "Notifications"},
    {Pinetime::Controllers::Settings::DoubleClickAction::AudioNext, "Audio Next"},
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

  uint32_t GetDefaultOption(Pinetime::Controllers::Settings::DoubleClickAction currentOption) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].action == currentOption) {
        return i;
      }
    }
    return 0;
  }
}

SettingDoubleClick::SettingDoubleClick(Pinetime::Controllers::Settings& settingsController)
  : checkboxList(
      0,
      1,
      "Double Click",
      Symbols::angleDoubleRight,
      GetDefaultOption(settingsController.GetDoubleClickAction()),
      [&settings = settingsController](uint32_t index) {
        settings.SetDoubleClickAction(options[index].action);
        settings.SaveSettings();
      },
      CreateOptionArray()) {
}

SettingDoubleClick::~SettingDoubleClick() {
  lv_obj_clean(lv_scr_act());
}
