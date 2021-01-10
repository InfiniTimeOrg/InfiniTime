#include "HeartRateController.h"
#include <heartratetask/HeartRateTask.h>

using namespace Pinetime::Controllers;

void HeartRateController::Update(HeartRateController::States newState, uint8_t heartRate) {
  this->state = newState;
  this->heartRate = heartRate;
}

void HeartRateController::Start() {
  if(task != nullptr)
    task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::StartMeasurement);
}

void HeartRateController::Stop() {
  if(task != nullptr)
    task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::StopMeasurement);
}

void HeartRateController::SetHeartRateTask(Pinetime::Applications::HeartRateTask *task) {
  this->task = task;
}
