#include "displayapp/screens/Clock.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include "components/battery/BatteryController.h"
#include "components/motion/MotionController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/settings/Settings.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/WatchFaceDigital.h"
#include "displayapp/screens/WatchFaceTerminal.h"
#include "displayapp/screens/WatchFaceAnalog.h"
#include "displayapp/screens/PineTimeStyle.h"
//
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
    motionController {motionController},
    screen {[this, &settingsController]() {
      switch (settingsController.GetClockFace()) {
        case 0:
          return WatchFaceDigitalScreen();
          break;
        case 1:
          return WatchFaceAnalogScreen();
          break;
        case 2:
          return PineTimeStyleScreen();
          break;
        case 3:
          return WatchFaceTerminalScreen();
          break;
      }
      return WatchFaceDigitalScreen();
    }()} {
  settingsController.SetAppMenu(0);
}

Clock::~Clock() {
  lv_obj_clean(lv_scr_act());
}

bool Clock::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screen->OnTouchEvent(event);
}

bool Clock::OnButtonPushed() {
  return screen->OnButtonPushed();
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

std::unique_ptr<Screen> Clock::PineTimeStyleScreen() {
  return std::make_unique<Screens::PineTimeStyle>(
    app, dateTimeController, batteryController, bleController, notificatioManager, settingsController, motionController);
}

std::unique_ptr<Screen> Clock::WatchFaceTerminalScreen() {
  return std::make_unique<Screens::WatchFaceTerminal>(app,
                                                      dateTimeController,
                                                      batteryController,
                                                      bleController,
                                                      notificatioManager,
                                                      settingsController,
                                                      heartRateController,
                                                      motionController);
}
