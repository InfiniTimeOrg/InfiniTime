#include "WatchFaceBase.h"
#include "components/datetime/DateTimeController.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"

using namespace Pinetime::Applications::Screens;

WatchFaceBase::WatchFaceBase(DisplayApp* app,
                             Controllers::DateTimeController const& dateTimeController,
                             Controllers::Battery const& batteryController,
                             Controllers::Ble const& bleController)
  : Screen{app},
    dateTimeController{dateTimeController},
    batteryController{batteryController},
    bleController{bleController} {
}

void WatchFaceBase::UpdateDate() {
  date = DateState{
    .year = dateTimeController.Year(),
    .month = dateTimeController.Month(),
    .day = dateTimeController.Day(),
    .dayOfWeek = dateTimeController.DayOfWeek()
  };
}

void WatchFaceBase::UpdateTime() {
  time = TimeState{
    .hour = dateTimeController.Hours(),
    .minute = dateTimeController.Minutes(),
    .second = dateTimeController.Seconds()
  };
}

void WatchFaceBase::UpdateBattery() {
  battery = BatteryState{
    .percentRemaining = batteryController.PercentRemaining(),
    .charging = batteryController.IsCharging(),
    .powerPresent = batteryController.IsPowerPresent()
  };
}

void WatchFaceBase::UpdateBle() {
  ble = BleState{
    .connected = bleController.IsConnected()
  };
}
