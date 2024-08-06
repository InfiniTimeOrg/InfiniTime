#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <components/heartrate/Ppg.h>
#include "components/settings/Settings.h"

#define DURATION_UNTIL_BACKGROUND_MEASUREMENT_IS_STOPPED pdMS_TO_TICKS(30 * 1000)

namespace Pinetime {
  namespace Drivers {
    class Hrs3300;
  }

  namespace Controllers {
    class HeartRateController;
  }

  namespace Applications {
    class HeartRateTask {
    public:
      enum class Messages : uint8_t {
        // Screen gets turned off
        GoToSleep,
        // Screen gets turned on
        WakeUp,
        // Start button pressed
        StartMeasurement,
        // Stop button pressed
        StopMeasurement
      };

      enum class States {
        // Screen turned off, heartrate not measured
        Idle,
        // Screen turned on, heartrate app open, heartrate not measured
        Running,
        // Screen turned on, heartrate app open, heartrate actively measured
        Measuring,
        // Screen turned off, heartrate task is waiting until the next measurement should be started
        BackgroundWaiting,
        // Screen turned off, heartrate actively measured
        BackgroundMeasuring
      };

      explicit HeartRateTask(Drivers::Hrs3300& heartRateSensor,
                             Controllers::HeartRateController& controller,
                             Controllers::Settings& settings);
      void Start();
      void Work();
      void PushMessage(Messages msg);

    private:
      static void Process(void* instance);
      void StartMeasurement();
      void StopMeasurement();
      void StartWaiting();

      void HandleGoToSleep();
      void HandleWakeUp();
      void HandleStartMeasurement(int* lastBpm);
      void HandleStopMeasurement();

      void HandleBackgroundWaiting();
      void HandleSensorData(int* lastBpm);

      TickType_t GetHeartRateBackgroundMeasurementIntervalInTicks();
      bool IsContinuosModeActivated();
      bool IsBackgroundMeasurementActivated();

      TaskHandle_t taskHandle;
      QueueHandle_t messageQueue;
      States state = States::Running;
      Drivers::Hrs3300& heartRateSensor;
      Controllers::HeartRateController& controller;
      Controllers::Settings& settings;
      Controllers::Ppg ppg;
      TickType_t backgroundWaitingStart = 0;
      TickType_t measurementStart = 0;
    };

  }
}
