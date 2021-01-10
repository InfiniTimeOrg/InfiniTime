#include "HeartRateController.h"
#include <heartratetask/HeartRateTask.h>
#include <systemtask/SystemTask.h>

using namespace Pinetime::Controllers;

HeartRateController::HeartRateController(Pinetime::System::SystemTask &systemTask) : systemTask{systemTask} {

}


void HeartRateController::Update(HeartRateController::States newState, uint8_t heartRate) {
  this->state = newState;
  this->heartRate = heartRate;
}

void HeartRateController::Start() {
  if(task != nullptr) {
    task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::StartMeasurement);
    systemTask.PushMessage(System::SystemTask::Messages::HeartRateRunning);
  }
}

void HeartRateController::Stop() {
  if(task != nullptr) {
    task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::StopMeasurement);
    systemTask.PushMessage(System::SystemTask::Messages::HeartRateStopped);
  }
}

void HeartRateController::SetHeartRateTask(Pinetime::Applications::HeartRateTask *task) {
  this->task = task;
}

