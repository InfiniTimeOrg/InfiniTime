#include "Clock.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "BatteryIcon.h"
#include "BleIcon.h"
#include "NotificationIcon.h"
#include "Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/motion/MotionController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "../DisplayApp.h"
#include "WatchFaceDigital.h"
#include "WatchFaceAnalog.h"

using namespace Pinetime::Applications::Screens;

Clock::Clock(DisplayApp* app,
             Controllers::DateTime& dateTimeController,
             Controllers::Battery& batteryController,
             Controllers::Ble& bleController,
             Controllers::NotificationManager& notificatioManager,
             Controllers::Settings& settingsController,
             Controllers::HeartRateController& heartRateController,
             Controllers::MotionController& motionController)
  : Screen(app),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController} {
  
  switch (settingsController.GetClockFace()) {
    case 0:
      screen = WatchFaceDigitalScreen();
      break;
    case 1:
      screen = WatchFaceAnalogScreen();
      break;
    /*
    // Examples for more watch faces
    case 2:
      screen = WatchFaceMinimalScreen();
      break;
    case 3:
      screen = WatchFaceCustomScreen();
      break;
    */
  }
  settingsController.SetAppMenu(0);
}

Clock::~Clock() {
  lv_obj_clean(lv_scr_act());
}

bool Clock::Refresh() {
  screen->Refresh();
  return running;
}

bool Clock::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screen->OnTouchEvent(event);
}

std::unique_ptr<Screen> Clock::WatchFaceDigitalScreen() {
  return std::make_unique<Screens::WatchFaceDigital>(app,
                                                     dateTimeController,
                                                     batteryController,
                                                     bleController,
                                                     notificatioManager,
                                                     settingsController,
                                                     heartRateController,
                                                     motionController);
}

std::unique_ptr<Screen> Clock::WatchFaceAnalogScreen() {
  return std::make_unique<Screens::WatchFaceAnalog>(
    app, dateTimeController, batteryController, bleController, notificatioManager, settingsController);
}

/*
// Examples for more watch faces
std::unique_ptr<Screen> Clock::WatchFaceMinimalScreen() {
  return std::make_unique<Screens::WatchFaceMinimal>(app, dateTimeController, batteryController, bleController, notificatioManager,
settingsController);
}

std::unique_ptr<Screen> Clock::WatchFaceCustomScreen() {
  return std::make_unique<Screens::WatchFaceCustom>(app, dateTimeController, batteryController, bleController, notificatioManager,
settingsController);
}
*/