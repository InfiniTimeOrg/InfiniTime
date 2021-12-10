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
    auto* controller = static_cast<Pinetime::Controllers::TimerController*> (p_context);
    if(controller != nullptr)
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
}

uint32_t TimerController::GetTimeRemaining() {
  if (!timerRunning) {
    return 0;
  }
  auto currentTicks = xTaskGetTickCount();
  
  TickType_t deltaTicks = 0;
  if (currentTicks > endTicks) {
    deltaTicks = 0xffffffff - currentTicks;
    deltaTicks += (endTicks + 1);
  } else {
    deltaTicks = endTicks - currentTicks;
  }
  
  return (static_cast<TickType_t>(deltaTicks) / static_cast<TickType_t>(configTICK_RATE_HZ)) * 1000;
}

void TimerController::StopTimer() {
  app_timer_stop(timerAppTimer);
  timerRunning = false;
}

bool TimerController::IsRunning() {
  return timerRunning;
}
void TimerController::OnTimerEnd() {
  timerRunning = false;
  if(systemTask != nullptr)
    systemTask->PushMessage(System::Messages::OnTimerDone);
}

void TimerController::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}
