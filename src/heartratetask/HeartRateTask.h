#pragma once
#include <FreeRTOS.h>
#include <cstdint>
#include <optional>
#include <task.h>
#include <queue.h>
#include "components/heartrate/Ppg.h"
#include "components/settings/Settings.h"
#include "components/heartrate/HeartRateController.h"

namespace Pinetime {
  namespace Drivers {
    class Hrs3300;
    class Bma421;
  }

  namespace Applications {
    class HeartRateTask {
    public:
      enum class Messages : uint8_t { GoToSleep, WakeUp, Enable, Disable };
      explicit HeartRateTask(Drivers::Hrs3300& heartRateSensor,
                             Controllers::HeartRateController& controller,
                             Controllers::Settings& settings,
                             Pinetime::Drivers::Bma421& motionSensor);
      void Start();
      void Work();
      void PushMessage(Messages msg);

    private:
      enum class States : uint8_t { Disabled, Waiting, BackgroundMeasuring, ForegroundMeasuring };
      static void Process(void* instance);
      void HandleSensorData();
      void StartMeasurement();
      void StopMeasurement();

      [[nodiscard]] bool BackgroundMeasurementNeeded() const;
      [[nodiscard]] std::optional<TickType_t> BackgroundMeasurementInterval() const;
      TickType_t CurrentTaskDelay();
      void SendHeartRate(Controllers::HeartRateController::States state, int bpm);

      TaskHandle_t taskHandle;
      QueueHandle_t messageQueue;
      bool valueCurrentlyShown;
      States state = States::Disabled;
      uint16_t count;
      Drivers::Hrs3300& heartRateSensor;
      Controllers::HeartRateController& controller;
      Controllers::Settings& settings;
      Drivers::Bma421& motionSensor;
      Controllers::Ppg ppg;
      TickType_t lastMeasurementTime;
      TickType_t measurementStartTime;
    };

  }
}
