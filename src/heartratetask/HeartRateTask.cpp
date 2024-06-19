#include "heartratetask/HeartRateTask.h"
#include <drivers/Hrs3300.h>
#include <components/heartrate/HeartRateController.h>
#include <nrf_log.h>

using namespace Pinetime::Applications;

HeartRateTask::HeartRateTask(Drivers::Hrs3300& heartRateSensor,
                             Controllers::HeartRateController& controller,
                             Controllers::Settings& settings)
  : heartRateSensor {heartRateSensor}, controller {controller}, settings {settings} {
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
    TickType_t delay = CurrentTaskDelay();
    Messages msg;

    if (xQueueReceive(messageQueue, &msg, delay) == pdTRUE) {
      switch (msg) {
        case Messages::GoToSleep:
          if (state == States::Running) {
            state = States::Idle;
          } else if (state == States::Measuring) {
            state = States::BackgroundWaiting;
            StartWaiting();
          }
          break;
        case Messages::WakeUp:
          if (state == States::Idle) {
            state = States::Running;
          } else if (state == States::BackgroundMeasuring) {
            state = States::Measuring;
          } else if (state == States::BackgroundWaiting) {
            state = States::Measuring;
            StartMeasurement();
          }
          break;
        case Messages::StartMeasurement:
          if (state == States::Measuring || state == States::BackgroundMeasuring) {
            break;
          }
          state = States::Measuring;
          lastBpm = 0;
          StartMeasurement();
          break;
        case Messages::StopMeasurement:
          if (state == States::Running || state == States::Idle) {
            break;
          }
          if (state == States::Measuring) {
            state = States::Running;
          } else if (state == States::BackgroundMeasuring) {
            state = States::Idle;
          }
          StopMeasurement();
          break;
      }
    }

    if (state == States::BackgroundWaiting) {
      HandleBackgroundWaiting();
    } else if (state == States::BackgroundMeasuring || state == States::Measuring) {
      HandleSensorData(&lastBpm);
    }
  }
}

void HeartRateTask::PushMessage(HeartRateTask::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(messageQueue, &msg, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HeartRateTask::StartMeasurement() {
  heartRateSensor.Enable();
  ppg.Reset(true);
  vTaskDelay(100);
  measurementStart = xTaskGetTickCount();
}

void HeartRateTask::StopMeasurement() {
  heartRateSensor.Disable();
  ppg.Reset(true);
  vTaskDelay(100);
}

void HeartRateTask::StartWaiting() {
  StopMeasurement();
  backgroundWaitingStart = xTaskGetTickCount();
}

void HeartRateTask::HandleBackgroundWaiting() {
  if (!IsBackgroundMeasurementActivated()) {
    return;
  }

  TickType_t ticksSinceWaitingStart = xTaskGetTickCount() - backgroundWaitingStart;
  if (ticksSinceWaitingStart >= GetHeartRateBackgroundMeasurementIntervalInTicks()) {
    state = States::BackgroundMeasuring;
    StartMeasurement();
  }
}

void HeartRateTask::HandleSensorData(int* lastBpm) {
  int8_t ambient = ppg.Preprocess(heartRateSensor.ReadHrs(), heartRateSensor.ReadAls());
  int bpm = ppg.HeartRate();

  // If ambient light detected or a reset requested (bpm < 0)
  if (ambient > 0) {
    // Reset all DAQ buffers
    ppg.Reset(true);
  } else if (bpm < 0) {
    // Reset all DAQ buffers except HRS buffer
    ppg.Reset(false);
    // Set HR to zero and update
    bpm = 0;
  }

  if (*lastBpm == 0 && bpm == 0) {
    controller.Update(Controllers::HeartRateController::States::NotEnoughData, bpm);
  }

  if (bpm != 0) {
    *lastBpm = bpm;
    controller.Update(Controllers::HeartRateController::States::Running, bpm);
    if (state == States::Measuring || IsContinuosModeActivated()) {
      return;
    }
    if (state == States::BackgroundMeasuring) {
      state = States::BackgroundWaiting;
      StartWaiting();
    }
  }
  TickType_t ticksSinceMeasurementStart = xTaskGetTickCount() - measurementStart;
  if (bpm == 0 && state == States::BackgroundMeasuring && !IsContinuosModeActivated() &&
      ticksSinceMeasurementStart >= DURATION_UNTIL_BACKGROUND_MEASURMENT_IS_STOPPED) {
    state = States::BackgroundWaiting;
    StartWaiting();
  }
  if (bpm == 0 && state == States::BackgroundMeasuring &&
      xTaskGetTickCount() - measurementStart >= DURATION_UNTIL_BACKGROUND_MEASURMENT_IS_STOPPED) {
    state = States::BackgroundWaiting;
    StartWaiting();
  }
}

TickType_t HeartRateTask::CurrentTaskDelay() {
  switch (state) {
    case States::Measuring:
    case States::BackgroundMeasuring:
      return ppg.deltaTms;
    case States::Running:
      return pdMS_TO_TICKS(100);
    case States::BackgroundWaiting:
      return pdMS_TO_TICKS(10000);
    default:
      return portMAX_DELAY;
  }
}

TickType_t HeartRateTask::GetHeartRateBackgroundMeasurementIntervalInTicks() {
  switch (settings.GetHeartRateBackgroundMeasurementInterval()) {
    case Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::TenSeconds:
      return pdMS_TO_TICKS(10 * 1000);
    case Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::ThirtySeconds:
      return pdMS_TO_TICKS(30 * 1000);
    case Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::OneMinute:
      return pdMS_TO_TICKS(60 * 1000);
    case Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::FiveMinutes:
      return pdMS_TO_TICKS(5 * 60 * 1000);
    case Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::TenMinutes:
      return pdMS_TO_TICKS(10 * 60 * 1000);
    case Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::ThirtyMinutes:
      return pdMS_TO_TICKS(30 * 60 * 1000);
    default:
      return 0;
  }
}

bool HeartRateTask::IsContinuosModeActivated() {
  return settings.GetHeartRateBackgroundMeasurementInterval() ==
         Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::Continuous;
}

bool HeartRateTask::IsBackgroundMeasurementActivated() {
  return settings.GetHeartRateBackgroundMeasurementInterval() !=
         Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::Off;
}
