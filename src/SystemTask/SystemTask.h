#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <memory>
#include <drivers/SpiMaster.h>
#include <drivers/St7789.h>
#include <Components/Battery/BatteryController.h>
#include <DisplayApp/DisplayApp.h>
#include <drivers/Watchdog.h>

namespace Pinetime {
  namespace System {
    class SystemTask {
      public:
        enum class Messages {GoToSleep, GoToRunning};

        SystemTask(Pinetime::Drivers::SpiMaster& spi,
                Pinetime::Drivers::St7789& lcd,
                Pinetime::Drivers::Cst816S& touchPanel,
                Pinetime::Components::LittleVgl& lvgl,
                Pinetime::Controllers::Battery& batteryController,
                Pinetime::Controllers::Ble& bleController,
                Pinetime::Controllers::DateTime& dateTimeController);


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


        static constexpr uint8_t pinSpiSck = 2;
        static constexpr uint8_t pinSpiMosi = 3;
        static constexpr uint8_t pinSpiMiso = 4;
        static constexpr uint8_t pinSpiCsn = 25;
        static constexpr uint8_t pinLcdDataCommand = 18;
        static constexpr uint8_t pinButton = 13;
        static constexpr uint8_t pinTouchIrq = 28;

        static void Process(void* instance);
        void Work();


    };
  }
}