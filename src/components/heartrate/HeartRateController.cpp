#include "components/heartrate/HeartRateController.h"

#include <cstdint>
#include "heartratetask/HeartRateTask.h"

using namespace Pinetime::Controllers;

void HeartRateController::UpdateState(HeartRateController::States newState) {
  this->state = newState;
}

void HeartRateController::UpdateHeartRate(uint8_t heartRate) {
  if (this->heartRate != heartRate) {
    this->heartRate = heartRate;
    service->OnNewHeartRateValue(heartRate);
  }
}

void HeartRateController::Enable() {
  if (task != nullptr) {
    state = States::Stopped;
    task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::Enable);
  }
}

void HeartRateController::Disable() {
  if (task != nullptr) {
    state = States::Disabled;
    task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::Disable);
  }
}

void HeartRateController::SetHeartRateTask(Pinetime::Applications::HeartRateTask* task) {
  this->task = task;
}

void HeartRateController::SetService(Pinetime::Controllers::HeartRateService* service) {
  this->service = service;
}
