#include "heartratetask/HeartRateTask.h"
#include <drivers/Hrs3300.h>
#include <components/heartrate/HeartRateController.h>
#include <nrf_log.h>

using namespace Pinetime::Applications;

TickType_t CurrentTaskDelay(HeartRateTask::States state, TickType_t ppgDeltaTms) {
  switch (state) {
    case HeartRateTask::States::ScreenOnAndMeasuring:
    case HeartRateTask::States::ScreenOffAndMeasuring:
      return ppgDeltaTms;
    case HeartRateTask::States::ScreenOffAndWaiting:
      return pdMS_TO_TICKS(1000);
    default:
      return portMAX_DELAY;
  }
}

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
    TickType_t delay = CurrentTaskDelay(state, ppg.deltaTms);
    Messages msg;

    if (xQueueReceive(messageQueue, &msg, delay) == pdTRUE) {
      switch (msg) {
        case Messages::GoToSleep:
          HandleGoToSleep();
          break;
        case Messages::WakeUp:
          HandleWakeUp();
          break;
        case Messages::StartMeasurement:
          HandleStartMeasurement(&lastBpm);
          break;
        case Messages::StopMeasurement:
          HandleStopMeasurement();
          break;
      }
    }

    switch (state) {
      case States::ScreenOffAndWaiting:
        HandleBackgroundWaiting();
        break;
      case States::ScreenOffAndMeasuring:
      case States::ScreenOnAndMeasuring:
        HandleSensorData(&lastBpm);
        break;
      case States::ScreenOffAndStopped:
      case States::ScreenOnAndStopped:
        // nothing to do -> ignore
        break;
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

void HeartRateTask::HandleGoToSleep() {
  switch (state) {
    case States::ScreenOnAndStopped:
      state = States::ScreenOffAndStopped;
      break;
    case States::ScreenOnAndMeasuring:
      if (IsBackgroundMeasurementActivated()) {
        state = States::ScreenOffAndMeasuring;
      } else {
        state = States::ScreenOffAndWaiting;
        StopMeasurement();
      }
      break;
    case States::ScreenOffAndStopped:
    case States::ScreenOffAndWaiting:
    case States::ScreenOffAndMeasuring:
      // shouldn't happen -> ignore
      break;
  }
}

void HeartRateTask::HandleWakeUp() {
  switch (state) {
    case States::ScreenOffAndStopped:
      state = States::ScreenOnAndStopped;
      break;
    case States::ScreenOffAndMeasuring:
      state = States::ScreenOnAndMeasuring;
      break;
    case States::ScreenOffAndWaiting:
      state = States::ScreenOnAndMeasuring;
      StartMeasurement();
      break;
    case States::ScreenOnAndStopped:
    case States::ScreenOnAndMeasuring:
      // shouldn't happen -> ignore
      break;
  }
}

void HeartRateTask::HandleStartMeasurement(int* lastBpm) {
  switch (state) {
    case States::ScreenOnAndStopped:
      state = States::ScreenOnAndMeasuring;
      *lastBpm = 0;
      StartMeasurement();
      break;
    case States::ScreenOffAndStopped:
    case States::ScreenOnAndMeasuring:
    case States::ScreenOffAndMeasuring:
    case States::ScreenOffAndWaiting:
      // shouldn't happen -> ignore
      break;
  }
}

void HeartRateTask::HandleStopMeasurement() {
  switch (state) {
    case States::ScreenOnAndMeasuring:
      state = States::ScreenOnAndStopped;
      StopMeasurement();
      break;
    case States::ScreenOffAndMeasuring:
    case States::ScreenOffAndWaiting:
    case States::ScreenOnAndStopped:
    case States::ScreenOffAndStopped:
      // shouldn't happen -> ignore
      break;
  }
}

void HeartRateTask::HandleBackgroundWaiting() {
  if (!IsBackgroundMeasurementActivated()) {
    return;
  }

  if (ShouldStartBackgroundMeasuring()) {
    state = States::ScreenOffAndMeasuring;
    StartMeasurement();
  }
}

void HeartRateTask::HandleSensorData(int* lastBpm) {
  auto sensorData = heartRateSensor.ReadHrsAls();
  int8_t ambient = ppg.Preprocess(sensorData.hrs, sensorData.als);
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

  bool notEnoughData = *lastBpm == 0 && bpm == 0;
  if (notEnoughData) {
    controller.Update(Controllers::HeartRateController::States::NotEnoughData, bpm);
  }

  if (bpm != 0) {
    *lastBpm = bpm;
    controller.Update(Controllers::HeartRateController::States::Running, bpm);
  }

  if (state == States::ScreenOnAndMeasuring || IsContinuousModeActivated()) {
    return;
  }

  // state == States::ScreenOffAndMeasuring
  //    (because state != ScreenOnAndMeasuring and the only state that enables measuring is ScreenOffAndMeasuring)
  // !IsContinuousModeActivated()

  if (ShouldStartBackgroundMeasuring()) {
    // This doesn't change the state but resets the measurment timer, which basically starts the next measurment without resetting the
    // sensor. This is basically a fall back to continuous mode, when measurments take too long.
    measurementStart = xTaskGetTickCount();
    return;
  }

  bool noDataWithinTimeLimit = bpm == 0 && ShoudStopTryingToGetData();
  bool dataWithinTimeLimit = bpm != 0;
  if (dataWithinTimeLimit || noDataWithinTimeLimit) {
    state = States::ScreenOffAndWaiting;
    StopMeasurement();
  }
}

TickType_t HeartRateTask::GetBackgroundIntervalInTicks() {
  int ms = settings.GetHeartRateBackgroundMeasurementInterval() * 1000;
  return pdMS_TO_TICKS(ms);
}

bool HeartRateTask::IsContinuousModeActivated() {
  return settings.GetHeartRateBackgroundMeasurementInterval() == 0;
}

bool HeartRateTask::IsBackgroundMeasurementActivated() {
  return settings.IsHeartRateBackgroundMeasurementActivated();
}

TickType_t HeartRateTask::GetTicksSinceLastMeasurementStarted() {
  return xTaskGetTickCount() - measurementStart;
}

bool HeartRateTask::ShoudStopTryingToGetData() {
  return GetTicksSinceLastMeasurementStarted() >= DURATION_UNTIL_BACKGROUND_MEASUREMENT_IS_STOPPED;
}

bool HeartRateTask::ShouldStartBackgroundMeasuring() {
  return GetTicksSinceLastMeasurementStarted() >= GetBackgroundIntervalInTicks();
}