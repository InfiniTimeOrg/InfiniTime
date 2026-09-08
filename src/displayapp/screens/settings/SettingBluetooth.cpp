#include "displayapp/screens/settings/SettingBluetooth.h"
#include "displayapp/screens/settings/SettingBluetoothMain.h"
#include "displayapp/screens/settings/SettingBluetoothSecurity.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/ScreenList.h"
#include "components/settings/Settings.h"
#include "displayapp/widgets/DotIndicator.h"

using namespace Pinetime::Applications::Screens;

bool SettingBluetooth::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

SettingBluetooth::SettingBluetooth(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : app {app},
    settings {settingsController},
    screens {app,
             0,
             {[this]() -> std::unique_ptr<Screen> {
                return screenBluetoothMain();
              },
              [this]() -> std::unique_ptr<Screen> {
                return screenBluetoothSecurity();
              }},
             Screens::ScreenListModes::UpDown} {
}

std::unique_ptr<Screen> SettingBluetooth::screenBluetoothMain() {
  Widgets::DotIndicator dotIndicator(0, 2);
  dotIndicator.Create();
  return std::make_unique<Screens::SettingBluetoothMain>(app, settings);
}

std::unique_ptr<Screen> SettingBluetooth::screenBluetoothSecurity() {
  Widgets::DotIndicator dotIndicator(1, 2);
  dotIndicator.Create();
  return std::make_unique<Screens::SettingBluetoothSecurity>(app, settings);
}

SettingBluetooth::~SettingBluetooth() {
  lv_obj_clean(lv_scr_act());
}
