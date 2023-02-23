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

void TimerController::StartTimer(std::chrono::milliseconds duration) {
  xTimerChangePeriod(timer, pdMS_TO_TICKS(duration.count()), 0);
  xTimerStart(timer, 0);
}

std::chrono::milliseconds TimerController::GetTimeRemaining() {
  if (IsRunning()) {
    TickType_t remainingTime = xTimerGetExpiryTime(timer) - xTaskGetTickCount();
    return std::chrono::milliseconds(remainingTime * 1000 / configTICK_RATE_HZ);
  }
  return std::chrono::milliseconds(0);
}

void TimerController::StopTimer() {
  xTimerStop(timer, 0);
}

bool TimerController::IsRunning() {
  return (xTimerIsTimerActive(timer) == pdTRUE);
}

void TimerController::OnTimerEnd() {
  systemTask->PushMessage(System::Messages::OnTimerDone);
}
