#include "components/timetracker/TimeTrackerController.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

/*
void TimerCallback(TimerHandle_t xTimer) {
  auto* controller = static_cast<TimerController*>(pvTimerGetTimerID(xTimer));
  controller->OnTimerEnd();
}
*/
void TimeTrackerController::Demo() {
  printf("Demo %i\n", this->totals[0]);
  this->totals[0] += 1;
}

void TimeTrackerController::Init() {
  Reset();
}

void TimeTrackerController::Reset() {
}

void TimeTrackerController::StartTimer(uint32_t duration) {
}

uint32_t TimeTrackerController::GetTimeRemaining() {
  return 1;
}

void TimeTrackerController::StopTimer() {
}

bool TimeTrackerController::IsRunning() {
  return true;
}

void TimeTrackerController::OnTimerEnd() {
}
