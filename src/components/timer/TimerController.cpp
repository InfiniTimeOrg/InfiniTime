//
// Created by florian on 16.05.21.
//

#include "TimerController.h"
#include "systemtask/SystemTask.h"
#include "app_timer.h"
#include "task.h"

using namespace Pinetime::Controllers;


APP_TIMER_DEF(timerAppTimer);


TimerController::TimerController(System::SystemTask& systemTask) : systemTask {systemTask} {
}


void TimerController::Init() {
  app_timer_create(&timerAppTimer, APP_TIMER_MODE_SINGLE_SHOT, timerEnd);

}

void TimerController::StartTimer(uint32_t duration) {
  app_timer_stop(timerAppTimer);
  app_timer_start(timerAppTimer, APP_TIMER_TICKS(duration), this);
  //dont ask why this random scaling factor is here. i got a difference between the time set and the time displayed and it works out to
  //be exactly this linear factor
  //you might be wondering why im not simply using app_timer_cnt_get() here. I am too. It is in app_timer.h, but the compiler says it
  // doesnt exist
  endTime = ((static_cast<float >(xTaskGetTickCount()) / static_cast<float >(configTICK_RATE_HZ)) * 1000 + APP_TIMER_TICKS(duration)) -
      APP_TIMER_TICKS(duration) * 0.024;
  timerRunning = true;
}

uint32_t TimerController::GetTimeRemaining() {
  if (!timerRunning) {
    return 0;
  }
  uint32_t currentTime = (static_cast<uint32_t>(xTaskGetTickCount()) / static_cast<uint32_t>(configTICK_RATE_HZ)) * 1000;
  return endTime - currentTime;
}

void TimerController::timerEnd(void* p_context) {
  
  auto *controller = static_cast<Controllers::TimerController*> (p_context);
  controller->timerRunning = false;
  controller->systemTask.PushMessage(System::SystemTask::Messages::OnTimerDone);
}

void TimerController::StopTimer() {
  app_timer_stop(timerAppTimer);
  timerRunning = false;
}

bool TimerController::IsRunning() {
  return timerRunning;
}