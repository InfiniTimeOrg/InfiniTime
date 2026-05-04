#include "heartratetask/HeartRateTask.h"
#include <drivers/Hrs3300.h>
#include <components/heartrate/HeartRateController.h>
#include <limits>

#include "utility/Math.h"

using namespace Pinetime::Applications;

namespace {
  constexpr TickType_t backgroundMeasurementTimeLimit = 30 * configTICK_RATE_HZ;
}

std::optional<TickType_t> HeartRateTask::BackgroundMeasurementInterval() const {
  auto interval = settings.GetHeartRateBackgroundMeasurementInterval();
  if (!interval.has_value()) {
    return std::nullopt;
  }
  return interval.value() * configTICK_RATE_HZ;
}

bool HeartRateTask::BackgroundMeasurementNeeded() const {
  auto backgroundPeriod = BackgroundMeasurementInterval();
  if (!backgroundPeriod.has_value()) {
    return false;
  }
  return xTaskGetTickCount() - lastMeasurementTime >= backgroundPeriod.value();
};

TickType_t HeartRateTask::CurrentTaskDelay() {
  auto backgroundPeriod = BackgroundMeasurementInterval();
  TickType_t currentTime = xTaskGetTickCount();
  auto CalculateSleepTicks = [&]() {
    TickType_t elapsed = currentTime - measurementStartTime;

    // Target system tick is the elapsed sensor ticks multiplied by the sensor tick duration (i.e. the elapsed time)
    // multiplied by the system tick rate
    // Since the sensor tick duration is a whole number of milliseconds, we compute in milliseconds and then divide by 1000
    // To avoid the number of milliseconds overflowing a u32, we take a factor of 2 out of the divisor and dividend
    // (1024 / 2) * 65536 * 100 = 3355443200 which is less than 2^32

    // Guard against future tick rate changes
    static_assert((configTICK_RATE_HZ / 2ULL) * (std::numeric_limits<decltype(count)>::max() + 1ULL) *
                      static_cast<uint64_t>((Pinetime::Controllers::Ppg::deltaTms)) <
                    std::numeric_limits<uint32_t>::max(),
                  "Overflow");
    TickType_t elapsedTarget = Utility::RoundedDiv(static_cast<uint32_t>(configTICK_RATE_HZ / 2) * (static_cast<uint32_t>(count) + 1U) *
                                                     static_cast<uint32_t>((Pinetime::Controllers::Ppg::deltaTms)),
                                                   static_cast<uint32_t>(1000 / 2));

    // On count overflow, reset both count and start time
    // Count is 16bit to avoid overflow in elapsedTarget
    // Count overflows every 100ms * u16 max = ~2 hours, much more often than the tick count (~48 days)
    // So no need to check for tick count overflow
    if (count == std::numeric_limits<decltype(count)>::max()) {
      count = 0;
      measurementStartTime = currentTime;
    }
    if (elapsedTarget > elapsed) {
      return elapsedTarget - elapsed;
    }
    return static_cast<TickType_t>(0);
  };
  switch (state) {
    case States::Disabled:
      return portMAX_DELAY;
    case States::Waiting:
      // Sleep until a new event if background measuring disabled
      if (!backgroundPeriod.has_value()) {
        return portMAX_DELAY;
      }
      // Sleep until the next background measurement
      if (currentTime - lastMeasurementTime < backgroundPeriod.value()) {
        return backgroundPeriod.value() - (currentTime - lastMeasurementTime);
      }
      // If one is due now, go straight away
      return 0;
    case States::BackgroundMeasuring:
    case States::ForegroundMeasuring:
      return CalculateSleepTicks();
  }
  // Needed to keep dumb compiler happy, this is unreachable
  // Any new additions to States will cause the above switch statement not to compile, so this is safe
  return portMAX_DELAY;
}

HeartRateTask::HeartRateTask(Drivers::Hrs3300& heartRateSensor,
                             Controllers::HeartRateController& controller,
                             Controllers::Settings& settings)
  : heartRateSensor {heartRateSensor}, controller {controller}, settings {settings} {
}

void HeartRateTask::Start() {
  messageQueue = xQueueCreate(10, 1);
  controller.SetHeartRateTask(this);

  if (pdPASS != xTaskCreate(HeartRateTask::Process, "Heartrate", 500, this, 1, &taskHandle)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

void HeartRateTask::Process(void* instance) {
  auto* app = static_cast<HeartRateTask*>(instance);
  app->Work();
}

void HeartRateTask::Work() {
  // measurementStartTime is always initialised before use by StartMeasurement
  // Need to initialise lastMeasurementTime so that the first background measurement happens at a reasonable time
  lastMeasurementTime = xTaskGetTickCount();
  valueCurrentlyShown = false;

  while (true) {
    TickType_t delay = CurrentTaskDelay();
    Messages msg;
    States newState = state;

    if (xQueueReceive(messageQueue, &msg, delay) == pdTRUE) {
      switch (msg) {
        case Messages::GoToSleep:
          // Ignore power state changes when disabled
          if (state == States::Disabled) {
            break;
          }
          // State is necessarily ForegroundMeasuring
          // As previously screen was on and measurement is enabled
          if (BackgroundMeasurementNeeded()) {
            newState = States::BackgroundMeasuring;
          } else {
            newState = States::Waiting;
          }
          break;
        case Messages::WakeUp:
          // Ignore power state changes when disabled
          if (state == States::Disabled) {
            break;
          }
          newState = States::ForegroundMeasuring;
          break;
        case Messages::Enable:
          // Can only be enabled when the screen is on
          // If this constraint is somehow violated, the unexpected state
          // will self-resolve at the next screen on event
          newState = States::ForegroundMeasuring;
          valueCurrentlyShown = false;
          break;
        case Messages::Disable:
          newState = States::Disabled;
          break;
      }
    }
    if (newState == States::Waiting && BackgroundMeasurementNeeded()) {
      newState = States::BackgroundMeasuring;
    } else if (newState == States::BackgroundMeasuring && !BackgroundMeasurementNeeded()) {
      newState = States::Waiting;
    }

    // Apply state transition (switch sensor on/off)
    if ((newState == States::ForegroundMeasuring || newState == States::BackgroundMeasuring) &&
        (state == States::Waiting || state == States::Disabled)) {
      StartMeasurement();
    } else if ((newState == States::Waiting || newState == States::Disabled) &&
               (state == States::ForegroundMeasuring || state == States::BackgroundMeasuring)) {
      StopMeasurement();
    }
    state = newState;

    if (state == States::ForegroundMeasuring || state == States::BackgroundMeasuring) {
      HandleSensorData();
      count++;
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
  measurementSucceeded = false;
  count = 0;
  measurementStartTime = xTaskGetTickCount();
}

void HeartRateTask::StopMeasurement() {
  heartRateSensor.Disable();
  ppg.Reset(true);
  vTaskDelay(100);
}

void HeartRateTask::HandleSensorData() {
  auto sensorData = heartRateSensor.ReadHrsAls();
  int8_t ambient = ppg.Preprocess(sensorData.hrs, sensorData.als);
  int bpm = ppg.HeartRate();

  // Ambient light detected
  if (ambient > 0) {
    // Reset all DAQ buffers
    ppg.Reset(true);
    controller.Update(Controllers::HeartRateController::States::NotEnoughData, bpm);
    bpm = 0;
    valueCurrentlyShown = false;
  }

  // Reset requested, or not enough data
  if (bpm == -1) {
    // Reset all DAQ buffers except HRS buffer
    ppg.Reset(false);
    // Set HR to zero and update
    bpm = 0;
    controller.Update(Controllers::HeartRateController::States::Running, bpm);
    valueCurrentlyShown = false;
  } else if (bpm == -2) {
    // Not enough data
    bpm = 0;
    if (!valueCurrentlyShown) {
      controller.Update(Controllers::HeartRateController::States::NotEnoughData, bpm);
    }
  }

  if (bpm != 0) {
    // Maintain constant frequency acquisition in background mode
    // If the last measurement time is set to the start time, then the next measurement
    // will start exactly one background period after this one
    // Avoid this if measurement exceeded the time limit (which happens with background intervals <= limit)
    if (state == States::BackgroundMeasuring && xTaskGetTickCount() - measurementStartTime < backgroundMeasurementTimeLimit) {
      lastMeasurementTime = measurementStartTime;
    } else {
      lastMeasurementTime = xTaskGetTickCount();
    }
    measurementSucceeded = true;
    valueCurrentlyShown = true;
    controller.Update(Controllers::HeartRateController::States::Running, bpm);
    return;
  }
  // If been measuring for longer than the time limit, set the last measurement time
  // This allows giving up on background measurement after a while
  // and also means that background measurement won't begin immediately after
  // an unsuccessful long foreground measurement
  if (xTaskGetTickCount() - measurementStartTime > backgroundMeasurementTimeLimit) {
    // When measuring, propagate failure if no value within the time limit
    // Prevents stale heart rates from being displayed for >1 background period
    // Or more than the time limit after switching to screen on (where the last background measurement was successful)
    // Note: Once a successful measurement is recorded in screen on it will never be cleared
    // without some other state change e.g. ambient light reset
    if (!measurementSucceeded) {
      controller.Update(Controllers::HeartRateController::States::Running, 0);
      valueCurrentlyShown = false;
    }
    if (state == States::BackgroundMeasuring) {
      lastMeasurementTime = xTaskGetTickCount() - backgroundMeasurementTimeLimit;
    } else {
      lastMeasurementTime = xTaskGetTickCount();
    }
  }
}
