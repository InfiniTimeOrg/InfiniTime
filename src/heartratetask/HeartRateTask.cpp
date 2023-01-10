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
    if (state == States::Running) {
      if (measurementStarted) {
        delay = 40;
      } else {
        delay = 100;
      }
    } else {
      delay = portMAX_DELAY;
    }

    Messages msg;
    if (xQueueReceive(messageQueue, &msg, delay) == pdTRUE) {
      switch (msg) {
        case Messages::GoToSleep:
          StopMeasurement();
          state = States::Idle;
          break;
        case Messages::WakeUp:
          state = States::Running;
          if (measurementStarted) {
            lastBpm = 0;
            StartMeasurement();
          }
          break;
        case Messages::StartMeasurement:
          if (measurementStarted) {
            break;
          }
          lastBpm = 0;
          StartMeasurement();
          measurementStarted = true;
          break;
        case Messages::StopMeasurement:
          if (!measurementStarted) {
            break;
          }
          StopMeasurement();
          measurementStarted = false;
          break;
      }
    }

    if (measurementStarted) {
      ppg.Preprocess(static_cast<float>(heartRateSensor.ReadHrs()));
      auto bpm = ppg.HeartRate();

      if (lastBpm == 0 && bpm == 0) {
        controller.Update(Controllers::HeartRateController::States::NotEnoughData, 0);
      }
      if (bpm != 0) {
        lastBpm = bpm;
        controller.Update(Controllers::HeartRateController::States::Running, lastBpm);
      }
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
  heartRateSensor.Enable();
  vTaskDelay(100);
  ppg.SetOffset(heartRateSensor.ReadHrs());
}

void HeartRateTask::StopMeasurement() {
  heartRateSensor.Disable();
  vTaskDelay(100);
}
