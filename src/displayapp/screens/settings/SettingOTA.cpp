#include "displayapp/screens/settings/SettingOTA.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/Messages.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    const char* name;
    Pinetime::Controllers::Settings::DfuAndFsMode mode;
  };

  constexpr std::array<Option, 3> options = {{
    {"Enabled", Pinetime::Controllers::Settings::DfuAndFsMode::Enabled},
    {"Disabled", Pinetime::Controllers::Settings::DfuAndFsMode::Disabled},
    {"Till reboot", Pinetime::Controllers::Settings::DfuAndFsMode::EnabledTillReboot},
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
  };
}

SettingOTA::SettingOTA(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app},
    settingsController {settingsController},
    checkboxList(
      0,
      1,
      "Firmware & files",
      Symbols::shieldAlt,
      settingsController.GetDfuAndFsMode() == Pinetime::Controllers::Settings::DfuAndFsMode::Enabled             ? 0
      : settingsController.GetDfuAndFsMode() == Pinetime::Controllers::Settings::DfuAndFsMode::EnabledTillReboot ? 2
                                                                                                                 : 1,
      [&settings = settingsController](uint32_t index) {
        settings.SetDfuAndFsMode(options[index].mode);
      },
      CreateOptionArray()) {
}

SettingOTA::~SettingOTA() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}
