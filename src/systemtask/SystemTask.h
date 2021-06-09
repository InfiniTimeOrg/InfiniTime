#pragma once

#include <memory>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <heartratetask/HeartRateTask.h>
#include <components/heartrate/HeartRateController.h>
#include <components/settings/Settings.h>
#include <drivers/Bma421.h>
#include <components/motion/MotionController.h>

#include "SystemMonitor.h"
#include "components/battery/BatteryController.h"
#include "components/ble/NimbleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motor/MotorController.h"
#include "components/timer/TimerController.h"
#ifdef PINETIME_IS_RECOVERY
  #include "displayapp/DisplayAppRecovery.h"
  #include "displayapp/DummyLittleVgl.h"
#else
  #include "components/settings/Settings.h"
  #include "displayapp/DisplayApp.h"
  #include "displayapp/LittleVgl.h"
#endif

#include "drivers/Watchdog.h"

namespace Pinetime {
  namespace Drivers {
    class Cst816S;
    class SpiMaster;
    class SpiNorFlash;
    class St7789;
    class TwiMaster;
    class Hrs3300;
  }
  namespace System {
    class SystemTask {
    public:
      enum class Messages {
        GoToSleep,
        GoToRunning,
        TouchWakeUp,
        OnNewTime,
        OnNewNotification,
        OnTimerDone,
        OnNewCall,
        BleConnected,
        UpdateTimeOut,
        BleFirmwareUpdateStarted,
        BleFirmwareUpdateFinished,
        OnTouchEvent,
        OnButtonEvent,
        OnDisplayTaskSleeping,
        EnableSleeping,
        DisableSleeping,
        OnNewDay,
        OnChargingEvent
      };

      SystemTask(Drivers::SpiMaster& spi,
                 Drivers::St7789& lcd,
                 Pinetime::Drivers::SpiNorFlash& spiNorFlash,
                 Drivers::TwiMaster& twiMaster,
                 Drivers::Cst816S& touchPanel,
                 Components::LittleVgl& lvgl,
                 Controllers::Battery& batteryController,
                 Controllers::Ble& bleController,
                 Pinetime::Controllers::MotorController& motorController,
                 Pinetime::Drivers::Hrs3300& heartRateSensor,
                 Pinetime::Drivers::Bma421& motionSensor,
                 Controllers::Settings& settingsController);

      void Start();
      void PushMessage(Messages msg);

      void OnButtonPushed();
      void OnTouchEvent();

      void OnIdle();

      Pinetime::Controllers::NimbleController& nimble() {
        return nimbleController;
      };

    private:
      TaskHandle_t taskHandle;

      Pinetime::Drivers::SpiMaster& spi;
      Pinetime::Drivers::St7789& lcd;
      Pinetime::Drivers::SpiNorFlash& spiNorFlash;
      Pinetime::Drivers::TwiMaster& twiMaster;
      Pinetime::Drivers::Cst816S& touchPanel;
      Pinetime::Components::LittleVgl& lvgl;
      Pinetime::Controllers::Battery& batteryController;
      std::unique_ptr<Pinetime::Applications::DisplayApp> displayApp;
      Pinetime::Controllers::HeartRateController heartRateController;
      std::unique_ptr<Pinetime::Applications::HeartRateTask> heartRateApp;

      Pinetime::Controllers::Ble& bleController;
      Pinetime::Controllers::DateTime dateTimeController;
      Pinetime::Controllers::TimerController timerController;
      QueueHandle_t systemTasksMsgQueue;
      std::atomic<bool> isSleeping {false};
      std::atomic<bool> isGoingToSleep {false};
      std::atomic<bool> isWakingUp {false};
      Pinetime::Drivers::Watchdog watchdog;
      Pinetime::Drivers::WatchdogView watchdogView;
      Pinetime::Controllers::NotificationManager notificationManager;
      Pinetime::Controllers::MotorController& motorController;
      Pinetime::Drivers::Hrs3300& heartRateSensor;
      Pinetime::Drivers::Bma421& motionSensor;
      Pinetime::Controllers::Settings& settingsController;
      Pinetime::Controllers::NimbleController nimbleController;
      Controllers::BrightnessController brightnessController;
      Pinetime::Controllers::MotionController motionController;

      static constexpr uint8_t pinSpiSck = 2;
      static constexpr uint8_t pinSpiMosi = 3;
      static constexpr uint8_t pinSpiMiso = 4;
      static constexpr uint8_t pinSpiCsn = 25;
      static constexpr uint8_t pinLcdDataCommand = 18;
      static constexpr uint8_t pinButton = 13;
      static constexpr uint8_t pinTouchIrq = 28;
      static constexpr uint8_t pinPowerPresentIrq = 19;

      static void Process(void* instance);
      void Work();
      void ReloadIdleTimer() const;
      bool isBleDiscoveryTimerRunning = false;
      uint8_t bleDiscoveryTimer = 0;
      TimerHandle_t idleTimer;
      bool doNotGoToSleep = false;

      void GoToRunning();
      void UpdateMotion();
      bool stepCounterMustBeReset = false;

#if configUSE_TRACE_FACILITY == 1
      SystemMonitor<FreeRtosMonitor> monitor;
#else
      SystemMonitor<DummyMonitor> monitor;
#endif
    };
  }
}
