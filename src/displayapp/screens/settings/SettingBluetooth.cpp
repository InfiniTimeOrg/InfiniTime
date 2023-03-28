#include "displayapp/screens/settings/SettingBluetooth.h"
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
    bool radioEnabled;
  };

  constexpr std::array<Option, 2> options = {{
    {"Enabled", true},
    {"Disabled", false},
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

SettingBluetooth::SettingBluetooth(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app},
    checkboxList(
      0,
      1,
      "Bluetooth",
      Symbols::bluetooth,
      settingsController.GetBleRadioEnabled() ? 0 : 1,
      [&settings = settingsController](uint32_t index) {
        const bool priorMode = settings.GetBleRadioEnabled();
        const bool newMode = options[index].radioEnabled;
        if (newMode != priorMode) {
          settings.SetBleRadioEnabled(newMode);
        }
      },
      CreateOptionArray()) {
}

SettingBluetooth::~SettingBluetooth() {
  lv_obj_clean(lv_scr_act());
  // Pushing the message in the OnValueChanged function causes a freeze?
  app->PushMessage(Pinetime::Applications::Display::Messages::BleRadioEnableToggle);
}
