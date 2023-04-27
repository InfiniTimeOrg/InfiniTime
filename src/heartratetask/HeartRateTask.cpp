#include "heartratetask/HeartRateTask.h"
#include <drivers/Hrs3300.h>
#include <components/heartrate/HeartRateController.h>
#include <nrf_log.h>

using namespace Pinetime::Applications;

HeartRateTask::HeartRateTask(Drivers::Hrs3300& heartRateSensor, Controllers::HeartRateController& controller)
  : heartRateSensor {heartRateSensor}, controller {controller} {
}

void HeartRateTask::Start() {
  messageQueue = xQueueCreate(10, 1);
  controller.SetHeartRateTask(this);

  if (pdPASS != xTaskCreate(HeartRateTask::Process, "Heartrate", 500, this, 0, &taskHandle)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

void HeartRateTask::Process(void* instance) {
  auto* app = static_cast<HeartRateTask*>(instance);
  app->Work();
}

void HeartRateTask::Work() {
  int lastBpm = 0;
  while (true) {
    auto delay = portMAX_DELAY;
    if (measuring) {
      delay = 40;
    }

    Messages msg;
    if (xQueueReceive(messageQueue, &msg, delay) == pdTRUE) {
      switch (msg) {
        case Messages::GoToSleep:
          sleeping = true;
          break;
        case Messages::WakeUp:
          sleeping = false;
          break;
        case Messages::StartMeasurementAwake:
          measureWhenAwake = true;
          break;
        case Messages::StopMeasurementAwake:
          measureWhenAwake = false;
          break;
        case Messages::StartMeasurementAlways:
          measureAlways = true;
          break;
        case Messages::StopMeasurementAlways:
          measureAlways = false;
          break;
      }
    }

    bool shouldMeasure = measureAlways || (measureWhenAwake && !sleeping);
    if (shouldMeasure && !measuring) {
      lastBpm = 0;
      StartMeasurement();
    } else if (!shouldMeasure && measuring) {
      StopMeasurement();
    }

    if (measuring) {
      ppg.Preprocess(static_cast<float>(heartRateSensor.ReadHrs()));
      auto bpm = ppg.HeartRate();

      if (lastBpm == 0 && bpm == 0) {
        controller.Update(Controllers::HeartRateController::States::NotEnoughData, 0);
      }
      if (bpm != 0) {
        lastBpm = bpm;
        controller.Update(Controllers::HeartRateController::States::Running, lastBpm);
      }
    } else if (measureWhenAwake && sleeping) {
        controller.Update(Controllers::HeartRateController::States::PausedBySleep, 0);
    } else {
        controller.Update(Controllers::HeartRateController::States::Stopped, 0);
    }
  }
}

void HeartRateTask::PushMessage(HeartRateTask::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(messageQueue, &msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken == pdTRUE) {
    /* Actual macro used here is port specific. */
    // TODO : should I do something here?
  }
}

void HeartRateTask::StartMeasurement() {
  measuring = true;
  heartRateSensor.Enable();
  vTaskDelay(100);
  ppg.SetOffset(heartRateSensor.ReadHrs());
}

void HeartRateTask::StopMeasurement() {
  measuring = false;
  heartRateSensor.Disable();
  vTaskDelay(100);
}
