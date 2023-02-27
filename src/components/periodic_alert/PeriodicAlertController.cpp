#include "components/periodic_alert/PeriodicAlertController.h"

using namespace Pinetime::Controllers;

namespace {
void Alert(TimerHandle_t xTimer) {
    auto* controller = static_cast<PeriodicAlertController*>(
        pvTimerGetTimerID(xTimer));
    controller->Alert();
}
}

PeriodicAlertController::PeriodicAlertController(DateTime& dateTimeController,
                                                 Settings& settingsController,
                                                 MotorController& motorController)
: settingsController{settingsController}
, dateTimeController{dateTimeController}
, motorController{motorController} {
}

void PeriodicAlertController::Init(System::SystemTask* /*systemTask*/) {
  alertTimer = xTimerCreate("PeriodicAlert", 1, pdFALSE, this, &::Alert);
  if (settingsController.GetPeriodicAlertEnabled()) {
    SetTimer();
  }
}

void PeriodicAlertController::SetPeriodicAlertMinute(uint8_t minute) {
  this->minute = minute;
}

void PeriodicAlertController::Enable() {
  if (settingsController.GetPeriodicAlertEnabled() &&
      settingsController.GetPeriodicAlertMinute() == minute) {
    return;
  }
  settingsController.SetPerioidcAlertMinute(minute);
  SetTimer();
}

void PeriodicAlertController::Disable() {
  if (!settingsController.GetPeriodicAlertEnabled()) {
    return;
  }
  settingsController.DisablePeriodicAlertMinute();
  xTimerStop(alertTimer, 0);
}

void PeriodicAlertController::SetTimer() {
  using namespace std::chrono;
  xTimerStop(alertTimer, 0);

  if (!settingsController.GetPeriodicAlertEnabled())
    return;

  const auto now = dateTimeController.CurrentDateTime();
  time_t ttAlertTime = system_clock::to_time_t(
      time_point_cast<system_clock::duration>(now));
  tm* tmAlertTime = std::localtime(&ttAlertTime);

  if (tmAlertTime->tm_min >= minute) {
    if (tmAlertTime->tm_hour >= 23) {
      tmAlertTime->tm_mday += 1;
      tmAlertTime->tm_wday = (tmAlertTime->tm_wday + 1) % 7;
    }
    tmAlertTime->tm_hour = (tmAlertTime->tm_hour + 1) % 24;
  }
  tmAlertTime->tm_min = minute;
  tmAlertTime->tm_sec = 0;
  tmAlertTime->tm_isdst = -1; // use system timezone setting to determine DST

  const auto alertTime = system_clock::from_time_t(std::mktime(tmAlertTime));
  const auto secondsToAlarm = duration_cast<seconds>(alertTime - now).count();
  xTimerChangePeriod(alertTimer, secondsToAlarm * configTICK_RATE_HZ, 0);
  xTimerStart(alertTimer, 0);
}

void PeriodicAlertController::Alert() {
  motorController.RunForDuration(35);
  SetTimer();
}
