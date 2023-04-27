#include "components/heartrate/HeartRateController.h"
#include <heartratetask/HeartRateTask.h>
#include <systemtask/SystemTask.h>

using namespace Pinetime::Controllers;

void HeartRateController::Update(HeartRateController::States newState, uint8_t heartRate) {
  this->state = newState;
  if (this->heartRate != heartRate) {
    this->heartRate = heartRate;
    service->OnNewHeartRateValue(heartRate);
  }
}

void HeartRateController::Start(bool measureInSleep) {
  if (task != nullptr) {
    if (measureInSleep) {
      task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::StartMeasurementAlways);
    } else {
      task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::StartMeasurementAwake);
    }
  }
}

void HeartRateController::Stop(bool measureInSleep) {
  if (task != nullptr) {
    if (measureInSleep) {
      task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::StopMeasurementAlways);
    } else {
      task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::StopMeasurementAwake);
    }
  }
}

void HeartRateController::SetHeartRateTask(Pinetime::Applications::HeartRateTask* task) {
  this->task = task;
}

void HeartRateController::SetService(Pinetime::Controllers::HeartRateService* service) {
  this->service = service;
}
