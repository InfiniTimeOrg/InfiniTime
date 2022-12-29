#include "components/timer/TimerController.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

void TimerCallback(TimerHandle_t xTimer) {
  auto* controller = static_cast<TimerController*>(pvTimerGetTimerID(xTimer));
  controller->OnTimerEnd();
}

void TimerController::Init(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
  timer = xTimerCreate("Timer", 1, pdFALSE, this, TimerCallback);
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
    case TimerState::Alerting:
      remainingTime = xTaskGetTickCount() - xTimerGetExpiryTime(timer);
      break;
  }
  return (remainingTime * 1000 / configTICK_RATE_HZ);
}

void TimerController::StopTimer() {
  xTimerStop(timer, 0);
  state = TimerState::Stopped;
}

void TimerController::OnTimerEnd() {
  state = TimerState::Alerting;
  systemTask->PushMessage(System::Messages::OnTimerDone);
}

void TimerController::StopAlerting() {
  state = TimerState::Stopped;
  systemTask->PushMessage(System::Messages::StopRinging);
}
