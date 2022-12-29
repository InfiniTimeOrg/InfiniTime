#include "components/timer/TimerController.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

void TimerCallback(TimerHandle_t xTimer) {
  auto* controller = static_cast<TimerController*>(pvTimerGetTimerID(xTimer));
  controller->OnTimerEnd();
}

void SetTimerToStopRingingCallback(TimerHandle_t xTimer) {
  auto* controller = static_cast<TimerController*>(pvTimerGetTimerID(xTimer));
  controller->StopRinging();
}

void TimerController::Init(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
  timer = xTimerCreate("Timer", 1, pdFALSE, this, TimerCallback);
  stopRingingTimer = xTimerCreate("StopRingingTimer", 1, pdFALSE, this, SetTimerToStopRingingCallback);
}

void TimerController::StartTimer(uint32_t duration) {
  xTimerChangePeriod(timer, pdMS_TO_TICKS(duration), 0);
  xTimerStart(timer, 0);
  state = TimerState::Running;
}

uint32_t TimerController::GetTimeRemaining() {
  TickType_t remainingTime = 0;
  switch (state) {
    case TimerState::Stopped:
      break;
    case TimerState::Running:
      remainingTime = xTimerGetExpiryTime(timer) - xTaskGetTickCount();
      break;
    case TimerState::Finished:
      remainingTime = xTaskGetTickCount() - xTimerGetExpiryTime(timer);
      break;
  }
  return (remainingTime * 1000 / configTICK_RATE_HZ);
}

void TimerController::StopTimer() {
  xTimerStop(timer, 0);
  if (state == TimerState::Finished) {
    StopRinging();
  }
  state = TimerState::Stopped;
}

void TimerController::OnTimerEnd() {
  state = TimerState::Finished;
  systemTask->PushMessage(System::Messages::OnTimerDone);
  ringing = true;
  xTimerChangePeriod(stopRingingTimer, pdMS_TO_TICKS(10000), 0);
  xTimerStart(stopRingingTimer, 0);
}

void TimerController::StopRinging() {
  if (ringing) {
    systemTask->PushMessage(System::Messages::OnTimerDoneRinging);
    ringing = false;
  }
}
