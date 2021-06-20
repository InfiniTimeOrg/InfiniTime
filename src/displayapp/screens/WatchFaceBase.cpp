#include "WatchFaceBase.h"
#include "components/datetime/DateTimeController.h"

using namespace Pinetime::Applications::Screens;

WatchFaceBase::WatchFaceBase(DisplayApp* app,
                             Controllers::DateTimeController const& dateTimeController)
  : Screen(app),
    dateTimeController(dateTimeController) {
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
