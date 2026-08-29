#include "displayapp/screens/settings/SettingQuickAccess.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <array>

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    Pinetime::Applications::Apps app;
    const char* name;
  };

  constexpr std::array<Option, 4> options = {{
    {Pinetime::Applications::Apps::None, "None"},
    {Pinetime::Applications::Apps::Music, "Music"},
    {Pinetime::Applications::Apps::HeartRate, "Heart Rate"},
    {Pinetime::Applications::Apps::StopWatch, "Stopwatch"},
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

  uint32_t GetDefaultOption(Pinetime::Applications::Apps currentApp) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].app == currentApp) {
        return i;
      }
    }
    return 0;
  }
}

SettingQuickAccess::SettingQuickAccess(Pinetime::Controllers::Settings& settingsController)
  : checkboxList(
      0,
      1,
      "Quick Access",
      Symbols::list,
      GetDefaultOption(settingsController.GetQuickAccessApp()),
      [&settings = settingsController](uint32_t index) {
        settings.SetQuickAccessApp(options[index].app);
        settings.SaveSettings();
      },
      CreateOptionArray()) {
}

SettingQuickAccess::~SettingQuickAccess() {
  lv_obj_clean(lv_scr_act());
}
