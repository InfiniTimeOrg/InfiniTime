#include "displayapp/DisplayAppRecovery.h"
#include <FreeRTOS.h>
#include <task.h>
#include <libraries/log/nrf_log.h>
#include "components/fs/FS.h"
#include "components/rle/RleDecoder.h"
#include "touchhandler/TouchHandler.h"
#include "displayapp/icons/infinitime/infinitime-nb.c"
#include "components/ble/BleController.h"

using namespace Pinetime::Applications;

DisplayApp::DisplayApp(Drivers::St7789& lcd,
                       const Drivers::Cst816S& /*touchPanel*/,
                       const Controllers::Battery& /*batteryController*/,
                       const Controllers::Ble& bleController,
                       Controllers::DateTime& /*dateTimeController*/,
                       const Drivers::Watchdog& /*watchdog*/,
                       Pinetime::Controllers::NotificationManager& /*notificationManager*/,
                       Pinetime::Controllers::HeartRateController& /*heartRateController*/,
                       Controllers::Settings& /*settingsController*/,
                       Pinetime::Controllers::MotorController& /*motorController*/,
                       Pinetime::Controllers::MotionController& /*motionController*/,
                       Pinetime::Controllers::AlarmController& /*alarmController*/,
                       Pinetime::Controllers::BrightnessController& /*brightnessController*/,
                       Pinetime::Controllers::TouchHandler& /*touchHandler*/,
                       Pinetime::Controllers::FS& /*filesystem*/)
  : lcd {lcd}, bleController {bleController} {
}

void DisplayApp::Start() {
  msgQueue = xQueueCreate(queueSize, itemSize);
  if (pdPASS != xTaskCreate(DisplayApp::Process, "displayapp", 512, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
}

void DisplayApp::Process(void* instance) {
  auto* app = static_cast<DisplayApp*>(instance);
  NRF_LOG_INFO("displayapp task started!");

  // Send a dummy notification to unlock the lvgl display driver for the first iteration
  xTaskNotifyGive(xTaskGetCurrentTaskHandle());

  app->InitHw();
  while (true) {
    app->Refresh();
  }
}

void DisplayApp::InitHw() {
  DisplayLogo(colorWhite);
}

void DisplayApp::Refresh() {
  Display::Messages msg;
  if (xQueueReceive(msgQueue, &msg, 200)) {
    switch (msg) {
      case Display::Messages::UpdateBleConnection:
        if (bleController.IsConnected()) {
          DisplayLogo(colorBlue);
        } else {
          DisplayLogo(colorWhite);
        }
        break;
      case Display::Messages::BleFirmwareUpdateStarted:
        DisplayLogo(colorGreen);
        break;
      default:
        break;
    }
  }

  if (bleController.IsFirmwareUpdating()) {
    uint8_t percent =
      (static_cast<float>(bleController.FirmwareUpdateCurrentBytes()) / static_cast<float>(bleController.FirmwareUpdateTotalBytes())) *
      100.0f;
    switch (bleController.State()) {
      case Controllers::Ble::FirmwareUpdateStates::Running:
        DisplayOtaProgress(percent, colorWhite);
        break;
      case Controllers::Ble::FirmwareUpdateStates::Validated:
        DisplayOtaProgress(100, colorGreenSwapped);
        break;
      case Controllers::Ble::FirmwareUpdateStates::Error:
        DisplayOtaProgress(100, colorRedSwapped);
        break;
      default:
        break;
    }
  }
}

void DisplayApp::DisplayLogo(uint16_t color) {
  Pinetime::Tools::RleDecoder rleDecoder(infinitime_nb, sizeof(infinitime_nb), color, colorBlack);
  for (int i = 0; i < displayWidth; i++) {
    rleDecoder.DecodeNext(displayBuffer, displayWidth * bytesPerPixel);
    ulTaskNotifyTake(pdTRUE, 500);
    lcd.DrawBuffer(0, i, displayWidth, 1, reinterpret_cast<const uint8_t*>(displayBuffer), displayWidth * bytesPerPixel);
  }
}

void DisplayApp::DisplayOtaProgress(uint8_t percent, uint16_t color) {
  const uint8_t barHeight = 20;
  std::fill(displayBuffer, displayBuffer + (displayWidth * bytesPerPixel), color);
  for (int i = 0; i < barHeight; i++) {
    ulTaskNotifyTake(pdTRUE, 500);
    uint16_t barWidth = std::min(static_cast<float>(percent) * 2.4f, static_cast<float>(displayWidth));
    lcd.DrawBuffer(0, displayWidth - barHeight + i, barWidth, 1, reinterpret_cast<const uint8_t*>(displayBuffer), barWidth * bytesPerPixel);
  }
}

void DisplayApp::PushMessage(Display::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(msgQueue, &msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    /* Actual macro used here is port specific. */
    // TODO : should I do something here?
  }
}

void DisplayApp::Register(Pinetime::System::SystemTask* /*systemTask*/) {
}

void DisplayApp::Register(Pinetime::Controllers::WeatherService* /*weatherService*/) {
}

void DisplayApp::Register(Pinetime::Controllers::MusicService* /*musicService*/) {
}

void DisplayApp::Register(Pinetime::Controllers::NavigationService* /*NavigationService*/) {
}
