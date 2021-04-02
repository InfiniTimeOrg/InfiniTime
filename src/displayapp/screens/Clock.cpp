#include "Clock.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "BatteryIcon.h"
#include "BleIcon.h"
#include "NotificationIcon.h"
#include "Symbols.h"
#include "components/battery/BatteryController.h"
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
        Controllers::Settings &settingsController,
        Controllers::HeartRateController& heartRateController) : Screen(app),
        dateTimeController{dateTimeController}, batteryController{batteryController},
        bleController{bleController}, notificatioManager{notificatioManager},
        settingsController{settingsController},
        heartRateController{heartRateController},
        screens{app, 
          settingsController.GetClockFace(),
          {
                [this]() -> std::unique_ptr<Screen> { return WatchFaceDigitalScreen(); },
                [this]() -> std::unique_ptr<Screen> { return WatchFaceAnalogScreen(); },
                // Examples for more watch faces
                //[this]() -> std::unique_ptr<Screen> { return WatchFaceMinimalScreen(); },
                //[this]() -> std::unique_ptr<Screen> { return WatchFaceCustomScreen(); }
          },
          Screens::ScreenListModes::LongPress          
        } {

          settingsController.SetAppMenu(0);

        }

Clock::~Clock() {
  lv_obj_clean(lv_scr_act());
}


bool Clock::Refresh() {  
  screens.Refresh();  
  return running;
}

bool Clock::OnButtonPushed() {
  running = false;
  return false;
}

bool Clock::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> Clock::WatchFaceDigitalScreen() {  
  return std::make_unique<Screens::WatchFaceDigital>(app, dateTimeController, batteryController, bleController, notificatioManager, settingsController, heartRateController);
}

std::unique_ptr<Screen> Clock::WatchFaceAnalogScreen() {  
  return std::make_unique<Screens::WatchFaceAnalog>(app, dateTimeController, batteryController, bleController, notificatioManager, settingsController);
}

/*
// Examples for more watch faces
std::unique_ptr<Screen> Clock::WatchFaceMinimalScreen() {  
  return std::make_unique<Screens::WatchFaceMinimal>(app, dateTimeController, batteryController, bleController, notificatioManager, settingsController);
}

std::unique_ptr<Screen> Clock::WatchFaceCustomScreen() {  
  return std::make_unique<Screens::WatchFaceCustom>(app, dateTimeController, batteryController, bleController, notificatioManager, settingsController);
}
*/