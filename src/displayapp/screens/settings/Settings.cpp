#include "Settings.h"
#include <lvgl/lvgl.h>
#include <array>
#include "SettingsList.h"
#include "displayapp/Apps.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

Settings::Settings(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app),
    settingsController {settingsController},
    screens {app,
             settingsController.GetSettingsMenu(),
             {[this]() -> std::unique_ptr<Screen> {
                return CreateScreen1();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen2();
              }},
             Screens::ScreenListModes::UpDown} {
}

Settings::~Settings() {
  lv_obj_clean(lv_scr_act());
}

bool Settings::Refresh() {

  if (running)
    running = screens.Refresh();
  return running;
}

bool Settings::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> Settings::CreateScreen1() {

  std::array<Screens::List::Item, 4> applications {{
    {Symbols::sun, "Display", static_cast<void*>(new Apps(Apps::SettingDisplay))},
    {Symbols::clock, "Wake Up", static_cast<void*>(new Apps(Apps::SettingWakeUp))},
    {Symbols::clock, "Time format", static_cast<void*>(new Apps(Apps::SettingTimeFormat))},
    {Symbols::clock, "Watch face",static_cast<void*>(new Apps(Apps::SettingWatchFace))},
  }};

  return std::unique_ptr<Screen>(new Screens::SettingsList(0, 2, app, settingsController, applications));
}

std::unique_ptr<Screen> Settings::CreateScreen2() {

  std::array<Screens::List::Item, 4> applications {{
    {Symbols::shoe, "Steps", static_cast<void*>(new Apps(Apps::SettingSteps))},
    {Symbols::batteryHalf, "Battery", static_cast<void*>(new Apps(Apps::BatteryInfo))},
    {Symbols::check, "Firmware", static_cast<void*>(new Apps(Apps::FirmwareValidation))},
    {Symbols::list, "About", static_cast<void*>(new Apps(Apps::SysInfo))},
  }};

  return std::unique_ptr<Screen>(new Screens::SettingsList(1, 2, app, settingsController, applications));
}
