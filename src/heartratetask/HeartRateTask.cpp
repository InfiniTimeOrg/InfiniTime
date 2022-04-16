#include "heartratetask/HeartRateTask.h"
#include <drivers/Hrs3300.h>
#include <components/heartrate/HeartRateController.h>
#include <nrf_log.h>

using namespace Pinetime::Applications;

HeartRateTask::HeartRateTask(Drivers::Hrs3300& heartRateSensor, Controllers::HeartRateController& controller)
  : heartRateSensor {heartRateSensor}, controller {controller}, ppg {} {
}

void HeartRateTask::Start() {
  messageQueue = xQueueCreate(10, 1);
  controller.SetHeartRateTask(this);
  lastRegularMeasurementTick = xTaskGetTickCount();

  if (pdPASS != xTaskCreate(HeartRateTask::Process, "Heartrate", 500, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
}

void HeartRateTask::Process(void* instance) {
  auto* app = static_cast<HeartRateTask*>(instance);
  app->Work();
}

void HeartRateTask::Work() {
  int lastBpm = 0;
  while (true) {
    Messages msg;
    uint32_t delay;
    if (state == States::Running || regularMeasurementStarted) {
      if (measurementStarted) {
        delay = 40;
      } else {
        delay = 100;
      }
    } else {
      delay = portMAX_DELAY;
    }
    if (xQueueReceive(messageQueue, &msg, delay)) {
      switch (msg) {
        case Messages::GoToSleep:
          // we don't want the regular measurement to stop if the watch goes to sleep, although we do want it to stop if it was
          // started by a user, to save power
          if (regularMeasurementStarted) {
            break;
          }
          StopMeasurement();
          state = States::Idle;
          break;
        case Messages::WakeUp:
          state = States::Running;
          // if the user started a measurement it was paused by GoToSleep so needs to be started again. If there is a regular Measurement
          // it was never stopped and must not be started again (as that would all currently collected data)
          if (userStartedMeasurement && !regularMeasurementStarted) {
            lastBpm = 0;
            StartMeasurement();
          }
          break;
        case Messages::StartMeasurement:
          if (measurementStarted) {
            // the user started a measurement
            userStartedMeasurement = true;
            break;
          }
          lastBpm = 0;
          StartMeasurement();
          measurementStarted = true;
          break;
        case Messages::StopMeasurement:
          if (measurementStarted) {
            StopMeasurement();
          }

          // if the user stops a measurement manually the regular measurement is cancelled as well
          // keeping the sensor on even after pressing the stop button could be confusing (although it turns on a tenth of a second later
          // again anyway)
          measurementStarted = false;
          userStartedMeasurement = false;
          regularMeasurementStarted = false;
          break;
      }
    }
    // compare if a Measurement should be started
    if (
      // if the variable is set to zero the feature is disabled and no measurement is supposed to be started
      regularMeasurmentWait != 0 &&
      // check if enough seconds have passed since the last measurement
      ((xTaskGetTickCount() - lastRegularMeasurementTick) / configTICK_RATE_HZ) > regularMeasurmentWait && !regularMeasurementStarted) {
      regularMeasurementStarted = true;
      // if there currently is a measurement it must not be started again, as that would invalidate all currently collected data
      if (!measurementStarted) {
        lastBpm = 0;
        StartMeasurement();
        measurementStarted = true;
      }
    }

    if (measurementStarted) {
      ppg.Preprocess(static_cast<float>(heartRateSensor.ReadHrs()));
      auto bpm = ppg.HeartRate();

      if (lastBpm == 0 && bpm == 0)
        controller.Update(Controllers::HeartRateController::States::NotEnoughData, 0);
      if (bpm != 0) {
        lastBpm = bpm;
        // this method handles a ble service as well, so nothing has to be done with the collected heart rate
        controller.Update(Controllers::HeartRateController::States::Running, lastBpm);
        if (regularMeasurementStarted) {
          regularMeasurementStarted = false;
          lastRegularMeasurementTick = xTaskGetTickCount();
          // we only want to turn the sensor off, if the user isn't currently manually measuring the heart rate
          if (!userStartedMeasurement) {
            StopMeasurement();
            measurementStarted = false;
          }
        }
      }
    }
  }
}

void HeartRateTask::PushMessage(HeartRateTask::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(messageQueue, &msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    /* Actual macro used here is port specific. */
    // TODO : should I do something here?
  }
}

void HeartRateTask::StartMeasurement() {
  heartRateSensor.Enable();
  vTaskDelay(100);
  ppg.SetOffset(static_cast<float>(heartRateSensor.ReadHrs()));
}

void HeartRateTask::StopMeasurement() {
  heartRateSensor.Disable();
  vTaskDelay(100);
}
