#include "Settings.h"
#include <lvgl/lvgl.h>
#include <array>
#include "displayapp/screens/List.h"
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

  std::array<Screens::List::Applications, 4> applications {{
    {Symbols::sun, "Display", Apps::SettingDisplay},
    {Symbols::eye, "Wake Up", Apps::SettingWakeUp},
    {Symbols::clock, "Time format", Apps::SettingTimeFormat},
    {Symbols::home, "Watch face", Apps::SettingWatchFace},
  }};

  return std::make_unique<Screens::List>(0, 2, app, settingsController, applications);
}

std::unique_ptr<Screen> Settings::CreateScreen2() {

  std::array<Screens::List::Applications, 4> applications {{
    {Symbols::shoe, "Steps", Apps::SettingSteps},
    {Symbols::batteryHalf, "Battery", Apps::BatteryInfo},
    {Symbols::check, "Firmware", Apps::FirmwareValidation},
    {Symbols::list, "About", Apps::SysInfo},
  }};

  return std::make_unique<Screens::List>(1, 2, app, settingsController, applications);
}
