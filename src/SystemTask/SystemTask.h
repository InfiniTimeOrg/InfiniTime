#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <memory>
#include <drivers/SpiMaster.h>
#include <drivers/St7789.h>
#include <Components/Battery/BatteryController.h>
#include <DisplayApp/DisplayApp.h>
#include <drivers/Watchdog.h>
#include <Components/Ble/NimbleController.h>

namespace Pinetime {
  namespace System {
    class SystemTask {
      public:
        enum class Messages {GoToSleep, GoToRunning, OnNewTime, OnNewNotification, BleConnected
        };

        SystemTask(Drivers::SpiMaster &spi, Drivers::St7789 &lcd, Drivers::Cst816S &touchPanel,
                   Components::LittleVgl &lvgl,
                   Controllers::Battery &batteryController, Controllers::Ble &bleController,
                   Controllers::DateTime &dateTimeController,
                   Pinetime::Controllers::NotificationManager& manager);


        void Start();
        void PushMessage(Messages msg);

        void OnButtonPushed();
        void OnTouchEvent();
      private:
        TaskHandle_t taskHandle;

        Pinetime::Drivers::SpiMaster& spi;
        Pinetime::Drivers::St7789& lcd;
        Pinetime::Drivers::Cst816S& touchPanel;
        Pinetime::Components::LittleVgl& lvgl;
        Pinetime::Controllers::Battery& batteryController;
        std::unique_ptr<Pinetime::Applications::DisplayApp> displayApp;
        Pinetime::Controllers::Ble& bleController;
        Pinetime::Controllers::DateTime& dateTimeController;
        QueueHandle_t systemTaksMsgQueue;
        bool isSleeping = false;
        Pinetime::Drivers::Watchdog watchdog;
        Pinetime::Drivers::WatchdogView watchdogView;
        Pinetime::Controllers::NotificationManager& notificationManager;
        Pinetime::Controllers::NimbleController nimbleController;


        static constexpr uint8_t pinSpiSck = 2;
        static constexpr uint8_t pinSpiMosi = 3;
        static constexpr uint8_t pinSpiMiso = 4;
        static constexpr uint8_t pinSpiCsn = 25;
        static constexpr uint8_t pinLcdDataCommand = 18;
        static constexpr uint8_t pinButton = 13;
        static constexpr uint8_t pinTouchIrq = 28;

        static void Process(void* instance);
        void Work();
        bool isBleDiscoveryTimerRunning = false;
        uint8_t bleDiscoveryTimer = 0;

    };
  }
}