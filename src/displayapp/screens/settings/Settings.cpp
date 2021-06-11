#include "Settings.h"
#include <lvgl/lvgl.h>
#include <array>
#include "displayapp/screens/List.h"
#include "displayapp/Apps.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "lv_i18n/lv_i18n.h" 

using namespace Pinetime::Applications::Screens;

Settings::Settings(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app),
    settingsController {settingsController},
    screens {app,
             settingsController.GetSettingsMenu(),
             {
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen1();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen2();
              },
	      [this]() -> std::unique_ptr<Screen> {
                return CreateScreen3();
	      },
	     },
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
    {Symbols::sun, _("settings_display"), Apps::SettingDisplay},
    {Symbols::clock, _("settings_wakeup"), Apps::SettingWakeUp},
    {Symbols::clock, _("settings_timeformat"), Apps::SettingTimeFormat},
    {Symbols::clock, _("settings_watchface"), Apps::SettingWatchFace},
  }};

  return std::make_unique<Screens::List>(0, 2, app, settingsController, applications);
}

std::unique_ptr<Screen> Settings::CreateScreen2() {

  std::array<Screens::List::Applications, 4> applications {{
    {Symbols::shoe, _("settings_steps"), Apps::SettingSteps},
    {Symbols::batteryHalf, _("settings_battery"), Apps::BatteryInfo},
    {Symbols::check, _("settings_firmware"), Apps::FirmwareValidation},
    {Symbols::list, _("settings_about"), Apps::SysInfo},
  }};

  return std::make_unique<Screens::List>(1, 2, app, settingsController, applications);
}

std::unique_ptr<Screen> Settings::CreateScreen3() {

  std::array<Screens::List::Applications, 4> applications {{
    {Symbols::list, _("settings_language"), Apps::SettingLanguage},
    {"", "", Apps::None},
    {"", "", Apps::None},
    {"", "", Apps::None},
  }};

  return std::unique_ptr<Screen>(new Screens::List(2, 2, app, settingsController, applications));
}
