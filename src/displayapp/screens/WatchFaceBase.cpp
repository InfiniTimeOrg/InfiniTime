#include "WatchFaceBase.h"
#include "components/datetime/DateTimeController.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"

using namespace Pinetime::Applications::Screens;

WatchFaceBase::WatchFaceBase(DisplayApp* app,
                             Controllers::DateTimeController const& dateTimeController,
                             Controllers::Battery const& batteryController,
                             Controllers::Ble const& bleController,
                             Controllers::NotificationManager const& notificationManager)
  : Screen{app},
    dateTimeController{dateTimeController},
    batteryController{batteryController},
    bleController{bleController},
    notificationManager{notificationManager} {
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

void WatchFaceBase::UpdateNotifications() {
  notifications = NotificationState{
    .newNotificationsAvailable = notificationManager.AreNewNotificationsAvailable()
  };
}
