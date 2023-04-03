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

void TimerController::StartTimer(std::chrono::hours hours, std::chrono::minutes minutes) {
  this->hours = hours;
  this->minutes = minutes;
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(hours)
                  + std::chrono::duration_cast<std::chrono::milliseconds>(minutes);
  StartTimer(duration);
}

void TimerController::StartTimer(std::chrono::milliseconds duration){
  state = TimerState::Running;
  auto durationSeconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
  xTimerChangePeriod(timer, durationSeconds.count() * configTICK_RATE_HZ, 0);
  xTimerStart(timer, 0);
}

std::chrono::milliseconds TimerController::GetTimeRemaining() {
  if (IsRunning()) {
    TickType_t remainingTime = xTimerGetExpiryTime(timer) - xTaskGetTickCount();
    auto remainingSeconds = std::chrono::seconds(remainingTime / configTICK_RATE_HZ);
    return std::chrono::duration_cast<std::chrono::milliseconds>(remainingSeconds);
  }
  return std::chrono::milliseconds(0);
}

void TimerController::StopTimer() {
  state = TimerState::Dormant;
  xTimerStop(timer, 0);
}

bool TimerController::IsRunning() {
  return (xTimerIsTimerActive(timer) == pdTRUE);
}

void TimerController::OnTimerEnd() {
  
  if(!useAlert){
    state = TimerState::Dormant;
    systemTask->PushMessage(System::Messages::OnTimerDone);
    return;
  }
  
  state = TimerState::Alerting;
  systemTask->PushMessage(System::Messages::StartTimerAlert);
}

void TimerController::SnoozeAlert(){
  if(state != TimerState::Alerting) return;
  state = TimerState::Snoozed;
  auto snoozeSeconds = std::chrono::duration_cast<std::chrono::seconds>(snoozeTime);
  xTimerChangePeriod(timer, snoozeSeconds.count() * configTICK_RATE_HZ, 0);
  xTimerStart(timer, 0);
}

void TimerController::StopAlerting() {
  StopTimer();
}
