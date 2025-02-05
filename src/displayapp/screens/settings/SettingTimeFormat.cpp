#include "displayapp/screens/settings/SettingTimeFormat.h"
#include "displayapp/screens/settings/SettingClockFormat.h"
#include "displayapp/screens/settings/SettingDateFormat.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/ScreenList.h"
#include "components/settings/Settings.h"
#include "displayapp/widgets/DotIndicator.h"

using namespace Pinetime::Applications::Screens;

bool SettingTimeFormat::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

SettingTimeFormat::SettingTimeFormat(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : settingsController {settingsController},
    screens {app,
             0,
             {[this]() -> std::unique_ptr<Screen> {
                return screenClockFormat();
              },
              [this]() -> std::unique_ptr<Screen> {
                return screenDateFormat();
              }},
             Screens::ScreenListModes::UpDown} {
}

std::unique_ptr<Screen> SettingTimeFormat::screenDateFormat() {
  Widgets::DotIndicator dotIndicator(1, 2);
  dotIndicator.Create();
  return std::make_unique<Screens::SettingDateFormat>(settingsController);
}

std::unique_ptr<Screen> SettingTimeFormat::screenClockFormat() {
  Widgets::DotIndicator dotIndicator(0, 2);
  dotIndicator.Create();
  return std::make_unique<Screens::SettingClockFormat>(settingsController);
}

SettingTimeFormat::~SettingTimeFormat() {
  lv_obj_clean(lv_scr_act());
}
