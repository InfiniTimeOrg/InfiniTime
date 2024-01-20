#include "displayapp/screens/settings/SettingSetSleepWakeTime.h"
#include "displayapp/screens/settings/SettingSetSleepTime.h"
#include "displayapp/screens/settings/SettingSetWakeTime.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/ScreenList.h"
#include "components/settings/Settings.h"
#include "displayapp/widgets/DotIndicator.h"

using namespace Pinetime::Applications::Screens;

bool SettingSetSleepWakeTime::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

SettingSetSleepWakeTime::SettingSetSleepWakeTime(Pinetime::Applications::DisplayApp* app,
                                                 Pinetime::Controllers::DateTime& dateTimeController,
                                                 Pinetime::Controllers::Settings& settingsController)
  : app {app},
    dateTimeController {dateTimeController},
    settingsController {settingsController},
    screens {app,
             0,
             {[this]() -> std::unique_ptr<Screen> {
                return screenSetSleepTime();
              },
              [this]() -> std::unique_ptr<Screen> {
                return screenSetWakeTime();
              }},
             Screens::ScreenListModes::UpDown} {
}

std::unique_ptr<Screen> SettingSetSleepWakeTime::screenSetSleepTime() {
  Widgets::DotIndicator dotIndicator(0, 2);
  dotIndicator.Create();
  return std::make_unique<Screens::SettingSetSleepTime>(dateTimeController, settingsController, *this);
}

std::unique_ptr<Screen> SettingSetSleepWakeTime::screenSetWakeTime() {
  Widgets::DotIndicator dotIndicator(1, 2);
  dotIndicator.Create();
  return std::make_unique<Screens::SettingSetWakeTime>(dateTimeController, settingsController, *this);
}

SettingSetSleepWakeTime::~SettingSetSleepWakeTime() {
  lv_obj_clean(lv_scr_act());
}

void SettingSetSleepWakeTime::Advance() {
  screens.OnTouchEvent(Pinetime::Applications::TouchEvents::SwipeUp);
}

void SettingSetSleepWakeTime::Quit() {
  running = false;
}
