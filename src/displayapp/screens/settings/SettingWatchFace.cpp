#include "displayapp/screens/settings/SettingWatchFace.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/CheckboxList.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

constexpr const char* SettingWatchFace::title;
constexpr const char* SettingWatchFace::symbol;

SettingWatchFace::SettingWatchFace(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app),
    settingsController {settingsController},
    screens {app,
             settingsController.GetWatchfacesMenu(),
             {[this]() -> std::unique_ptr<Screen> {
                return CreateScreen1();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen2();
              }},
             Screens::ScreenListModes::UpDown} {
}

SettingWatchFace::~SettingWatchFace() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

bool SettingWatchFace::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SettingWatchFace::CreateScreen1() {
  std::array<const char*, 4> watchfaces {" Digital face", " Analog face", " PineTimeStyle", " Terminal"};
  return std::make_unique<Screens::CheckboxList>(
    0, 2, app, settingsController, title, symbol, &Controllers::Settings::SetClockFace, &Controllers::Settings::GetClockFace, watchfaces);
}

std::unique_ptr<Screen> SettingWatchFace::CreateScreen2() {
  std::array<const char*, 4> watchfaces {" Casio G7710", "", "", ""};
  return std::make_unique<Screens::CheckboxList>(
    1, 2, app, settingsController, title, symbol, &Controllers::Settings::SetClockFace, &Controllers::Settings::GetClockFace, watchfaces);
}
