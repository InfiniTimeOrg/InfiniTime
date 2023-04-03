#include "displayapp/screens/settings/SettingSetDateTime.h"
#include "displayapp/screens/settings/SettingSetDate.h"
#include "displayapp/screens/settings/SettingSetTime.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/ScreenList.h"
#include "components/settings/Settings.h"
#include "displayapp/widgets/DotIndicator.h"

using namespace Pinetime::Applications::Screens;

bool SettingSetDateTime::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

SettingSetDateTime::SettingSetDateTime(Pinetime::Applications::DisplayApp* app,
                                       Pinetime::Controllers::DateTime& dateTimeController,
                                       Pinetime::Controllers::Settings& settingsController)
  : app {app},
    dateTimeController {dateTimeController},
    settingsController {settingsController},
    screens {app,
             0,
             {[this]() -> std::unique_ptr<Screen> {
                return screenSetDate();
              },
              [this]() -> std::unique_ptr<Screen> {
                return screenSetTime();
              }},
             Screens::ScreenListModes::UpDown} {
}

std::unique_ptr<Screen> SettingSetDateTime::screenSetDate() {
  Widgets::DotIndicator dotIndicator(0, 2);
  dotIndicator.Create();
  return std::make_unique<Screens::SettingSetDate>(dateTimeController, *this);
}

std::unique_ptr<Screen> SettingSetDateTime::screenSetTime() {
  Widgets::DotIndicator dotIndicator(1, 2);
  dotIndicator.Create();
  return std::make_unique<Screens::SettingSetTime>(dateTimeController, settingsController, *this);
}

SettingSetDateTime::~SettingSetDateTime() {
  lv_obj_clean(lv_scr_act());
}

void SettingSetDateTime::Advance() {
  screens.OnTouchEvent(Pinetime::Applications::TouchEvents::SwipeUp);
}

void SettingSetDateTime::Quit() {
  running = false;
}
