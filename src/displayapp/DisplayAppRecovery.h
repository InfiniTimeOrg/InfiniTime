#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <drivers/St7789.h>
#include <drivers/SpiMaster.h>
#include <bits/unique_ptr.h>
#include <queue.h>
#include "components/gfx/Gfx.h"
#include "components/battery/BatteryController.h"
#include "components/brightness/BrightnessController.h"
#include "components/ble/BleController.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/NotificationManager.h"
#include "components/firmwarevalidator/FirmwareValidator.h"
#include "drivers/Cst816s.h"
#include <date/date.h>
#include <drivers/Watchdog.h>
#include <components/heartrate/HeartRateController.h>
#include <components/motion/MotionController.h>
#include <components/motor/MotorController.h>
#include <components/settings/Settings.h>
#include "TouchEvents.h"
#include "Apps.h"
#include "Messages.h"
#include "DummyLittleVgl.h"
#include "components/timer/TimerController.h"

namespace Pinetime {
  namespace System {
    class SystemTask;
  };
  namespace Applications {
    class DisplayApp {
    public:
      DisplayApp(Drivers::St7789& lcd,
                 Components::LittleVgl& lvgl,
                 Drivers::Cst816S&,
                 Controllers::Battery& batteryController,
                 Controllers::Ble& bleController,
                 Controllers::DateTime& dateTimeController,
                 Drivers::WatchdogView& watchdog,
                 System::SystemTask& systemTask,
                 Pinetime::Controllers::NotificationManager& notificationManager,
                 Pinetime::Controllers::HeartRateController& heartRateController,
                 Controllers::Settings& settingsController,
                 Pinetime::Controllers::MotorController& motorController,
                 Pinetime::Controllers::MotionController& motionController,
                 Pinetime::Controllers::TimerController& timerController);
      void Start();
      void PushMessage(Pinetime::Applications::Display::Messages msg);

    private:
      TaskHandle_t taskHandle;
      static void Process(void* instance);
      void DisplayLogo(uint16_t color);
      void DisplayOtaProgress(uint8_t percent, uint16_t color);
      void InitHw();
      void Refresh();
      Pinetime::Drivers::St7789& lcd;
      Controllers::Ble& bleController;

      static constexpr uint8_t queueSize = 10;
      static constexpr uint8_t itemSize = 1;
      QueueHandle_t msgQueue;
      static constexpr uint8_t displayWidth = 240;
      static constexpr uint8_t displayHeight = 240;
      static constexpr uint8_t bytesPerPixel = 2;

      static constexpr uint16_t colorWhite = 0xFFFF;
      static constexpr uint16_t colorGreen = 0x07E0;
      static constexpr uint16_t colorGreenSwapped = 0xE007;
      static constexpr uint16_t colorBlue = 0x0000ff;
      static constexpr uint16_t colorRed = 0xff00;
      static constexpr uint16_t colorRedSwapped = 0x00ff;
      static constexpr uint16_t colorBlack = 0x0000;
      uint8_t displayBuffer[displayWidth * bytesPerPixel];
    };
  }
}
