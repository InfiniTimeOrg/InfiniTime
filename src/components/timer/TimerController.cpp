//
// Created by florian on 16.05.21.
//

#include "components/timer/TimerController.h"
#include "systemtask/SystemTask.h"
#include "app_timer.h"
#include "task.h"

using namespace Pinetime::Controllers;

APP_TIMER_DEF(timerAppTimer);

namespace {
  void TimerEnd(void* p_context) {
    auto* controller = static_cast<Pinetime::Controllers::TimerController*>(p_context);
    if (controller != nullptr)
      controller->OnTimerEnd();
  }
}

void TimerController::Init() {
  app_timer_create(&timerAppTimer, APP_TIMER_MODE_SINGLE_SHOT, TimerEnd);
}

void TimerController::StartTimer(uint32_t duration) {
  app_timer_stop(timerAppTimer);
  auto currentTicks = xTaskGetTickCount();
  app_timer_start(timerAppTimer, APP_TIMER_TICKS(duration), this);
  endTicks = currentTicks + APP_TIMER_TICKS(duration);
  timerRunning = true;
  overtime = false;
}

int32_t TimerController::GetSecondsRemaining() {
  if (!timerRunning) {
    return 0;
  }
  auto currentTicks = xTaskGetTickCount();

  int32_t deltaTicks = static_cast<int32_t>(endTicks) - static_cast<int32_t>(currentTicks);

  return (deltaTicks / static_cast<int32_t>(configTICK_RATE_HZ));
}

void TimerController::StopTimer() {
  app_timer_stop(timerAppTimer);
  timerRunning = false;
  overtime = false;
}

void TimerController::OnTimerEnd() {
  overtime = true;
  if (systemTask != nullptr) {
    systemTask->PushMessage(System::Messages::OnTimerDone);
  }
}

void TimerController::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}
