#include "displayapp/screens/settings/SettingNotificationTimeout.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    uint32_t notificationTimeout;
    const char* name;
  };

  constexpr std::array<Option, 3> options = {{
    {7000, "7s"},
    {15000, "15s"},
    {30000, "30s"},
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

  uint32_t GetDefaultOption(uint32_t currentOption) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].notificationTimeout == currentOption) {
        return i;
      }
    }
    return 0;
  }
}

SettingNotificationTimeout::SettingNotificationTimeout(Pinetime::Controllers::Settings& settingsController)
  : checkboxList(
      0,
      1,
      "Notification\nTimeout",
      Symbols::bell,
      GetDefaultOption(settingsController.GetNotificationTimeout()),
      [&settings = settingsController](uint32_t index) {
        settings.SetNotificationTimeout(options[index].notificationTimeout);
        settings.SaveSettings();
      },
      CreateOptionArray()) {
}

SettingNotificationTimeout::~SettingNotificationTimeout() {
  lv_obj_clean(lv_scr_act());
}
