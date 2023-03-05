#include "displayapp/screens/Clock.h"

#include <lvgl/lvgl.h>
#include "components/battery/BatteryController.h"
#include "components/motion/MotionController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/settings/Settings.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/WatchFaceDigital.h"
#include "displayapp/screens/WatchFaceTerminal.h"
#include "displayapp/screens/WatchFaceInfineat.h"
#include "displayapp/screens/WatchFaceAnalog.h"
#include "displayapp/screens/WatchFacePineTimeStyle.h"
#include "displayapp/screens/WatchFaceCasioStyleG7710.h"

using namespace Pinetime::Applications::Screens;

Clock::Clock(Controllers::DateTime& dateTimeController,
             const Controllers::Battery& batteryController,
             const Controllers::Ble& bleController,
             Controllers::NotificationManager& notificationManager,
             Controllers::Settings& settingsController,
             Controllers::HeartRateController& heartRateController,
             Controllers::MotionController& motionController,
             Controllers::FS& filesystem)
  : dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    filesystem {filesystem},
    screen {[this, &settingsController]() {
      switch (settingsController.GetClockFace()) {
        case 0:
          return WatchFaceDigitalScreen();
          break;
        case 1:
          return WatchFaceAnalogScreen();
          break;
        case 2:
          return WatchFacePineTimeStyleScreen();
          break;
        case 3:
          return WatchFaceTerminalScreen();
          break;
        case 4:
          return WatchFaceInfineatScreen();
          break;
        case 5:
          return WatchFaceCasioStyleG7710();
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
  return std::make_unique<Screens::WatchFaceDigital>(dateTimeController,
                                                     batteryController,
                                                     bleController,
                                                     notificationManager,
                                                     settingsController,
                                                     heartRateController,
                                                     motionController);
}

std::unique_ptr<Screen> Clock::WatchFaceAnalogScreen() {
  return std::make_unique<Screens::WatchFaceAnalog>(dateTimeController,
                                                    batteryController,
                                                    bleController,
                                                    notificationManager,
                                                    settingsController);
}

std::unique_ptr<Screen> Clock::WatchFacePineTimeStyleScreen() {
  return std::make_unique<Screens::WatchFacePineTimeStyle>(dateTimeController,
                                                           batteryController,
                                                           bleController,
                                                           notificationManager,
                                                           settingsController,
                                                           motionController);
}

std::unique_ptr<Screen> Clock::WatchFaceTerminalScreen() {
  return std::make_unique<Screens::WatchFaceTerminal>(dateTimeController,
                                                      batteryController,
                                                      bleController,
                                                      notificationManager,
                                                      settingsController,
                                                      heartRateController,
                                                      motionController);
}

std::unique_ptr<Screen> Clock::WatchFaceInfineatScreen() {
  return std::make_unique<Screens::WatchFaceInfineat>(dateTimeController,
                                                      batteryController,
                                                      bleController,
                                                      notificationManager,
                                                      settingsController,
                                                      motionController,
                                                      filesystem);
}

std::unique_ptr<Screen> Clock::WatchFaceCasioStyleG7710() {
  return std::make_unique<Screens::WatchFaceCasioStyleG7710>(dateTimeController,
                                                             batteryController,
                                                             bleController,
                                                             notificationManager,
                                                             settingsController,
                                                             heartRateController,
                                                             motionController,
                                                             filesystem);
}
