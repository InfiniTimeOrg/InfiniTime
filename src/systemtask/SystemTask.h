#pragma once

#include <memory>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <heartratetask/HeartRateTask.h>
#include <components/heartrate/HeartRateController.h>

#include "SystemMonitor.h"
#include "components/battery/BatteryController.h"
#include "components/ble/NimbleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motor/MotorController.h"
#include "displayapp/DisplayApp.h"
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
        enum class Messages {GoToSleep, GoToRunning, OnNewTime, OnNewNotification, OnNewCall, BleConnected,
            BleFirmwareUpdateStarted, BleFirmwareUpdateFinished, OnTouchEvent, OnButtonEvent, OnDisplayTaskSleeping
        };

        SystemTask(Drivers::SpiMaster &spi, Drivers::St7789 &lcd,
                   Pinetime::Drivers::SpiNorFlash& spiNorFlash,
                   Drivers::TwiMaster& twiMaster, Drivers::Cst816S &touchPanel,
                   Components::LittleVgl &lvgl,
                   Controllers::Battery &batteryController, Controllers::Ble &bleController,
                   Controllers::DateTime &dateTimeController,
                   Pinetime::Controllers::MotorController& motorController,
                   Pinetime::Drivers::Hrs3300& heartRateSensor);


        void Start();
        void PushMessage(Messages msg);

        void OnButtonPushed();
        void OnTouchEvent();

        void OnIdle();

        Pinetime::Controllers::NimbleController& nimble() {return nimbleController;};

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
        Pinetime::Controllers::DateTime& dateTimeController;
        QueueHandle_t systemTasksMsgQueue;
        std::atomic<bool> isSleeping{false};
        std::atomic<bool> isGoingToSleep{false};
        std::atomic<bool> isWakingUp{false};
        Pinetime::Drivers::Watchdog watchdog;
        Pinetime::Drivers::WatchdogView watchdogView;
        Pinetime::Controllers::NotificationManager notificationManager;
        Pinetime::Controllers::MotorController& motorController;
        Pinetime::Drivers::Hrs3300& heartRateSensor;
        Pinetime::Controllers::NimbleController nimbleController;

        static constexpr uint8_t pinSpiSck = 2;
        static constexpr uint8_t pinSpiMosi = 3;
        static constexpr uint8_t pinSpiMiso = 4;
        static constexpr uint8_t pinSpiCsn = 25;
        static constexpr uint8_t pinLcdDataCommand = 18;
        static constexpr uint8_t pinButton = 17;
        static constexpr uint8_t pinTouchIrq = 28;

        static void Process(void* instance);
        void Work();
        void ReloadIdleTimer() const;
        bool isBleDiscoveryTimerRunning = false;
        uint8_t bleDiscoveryTimer = 0;
        static constexpr uint32_t idleTime = 15000;
        TimerHandle_t idleTimer;
        bool doNotGoToSleep = false;

        void GoToRunning();

#if configUSE_TRACE_FACILITY == 1
        SystemMonitor<FreeRtosMonitor> monitor;
#else
        SystemMonitor<DummyMonitor> monitor;
#endif
    };
  }
}
