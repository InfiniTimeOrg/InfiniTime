#include <legacy/nrf_drv_clock.h>
#include <softdevice/common/nrf_sdh.h>
#include <drivers/SpiMaster.h>
#include <drivers/Spi.h>
#include <drivers/SpiNorFlash.h>
#include <libraries/log/nrf_log.h>
#include <FreeRTOS.h>
#include <task.h>
#include <legacy/nrf_drv_gpiote.h>
#include <libraries/gpiote/app_gpiote.h>
#include <hal/nrf_wdt.h>
#include <cstring>
#include <components/gfx/Gfx.h>
#include <drivers/St7789.h>
#include <components/brightness/BrightnessController.h>
#include <algorithm>
#include "recoveryImage.h"
#include "drivers/PinMap.h"

#include "displayapp/icons/infinitime/infinitime-nb.c"
#include "components/rle/RleDecoder.h"

#if NRF_LOG_ENABLED
  #include "logging/NrfLogger.h"
Pinetime::Logging::NrfLogger logger;
#else
  #include "logging/DummyLogger.h"
Pinetime::Logging::DummyLogger logger;
#endif

static constexpr uint8_t displayWidth = 240;
static constexpr uint8_t displayHeight = 240;
static constexpr uint8_t bytesPerPixel = 2;

static constexpr uint16_t colorWhite = 0xFFFF;
static constexpr uint16_t colorGreen = 0xE007;

Pinetime::Drivers::SpiMaster spi {Pinetime::Drivers::SpiMaster::SpiModule::SPI0,
                                  {Pinetime::Drivers::SpiMaster::BitOrder::Msb_Lsb,
                                   Pinetime::Drivers::SpiMaster::Modes::Mode3,
                                   Pinetime::Drivers::SpiMaster::Frequencies::Freq8Mhz,
                                   Pinetime::PinMap::SpiSck,
                                   Pinetime::PinMap::SpiMosi,
                                   Pinetime::PinMap::SpiMiso}};
Pinetime::Drivers::Spi flashSpi {spi, Pinetime::PinMap::SpiFlashCsn};
Pinetime::Drivers::SpiNorFlash spiNorFlash {flashSpi};

Pinetime::Drivers::Spi lcdSpi {spi, Pinetime::PinMap::SpiLcdCsn};
Pinetime::Drivers::St7789 lcd {lcdSpi, Pinetime::PinMap::LcdDataCommand, Pinetime::PinMap::LcdReset};

Pinetime::Components::Gfx gfx {lcd};
Pinetime::Controllers::BrightnessController brightnessController;

void DisplayProgressBar(uint8_t percent, uint16_t color);

void DisplayLogo();

extern "C" {
void vApplicationIdleHook(void) {
}

void SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler(void) {
  if (((NRF_SPIM0->INTENSET & (1 << 6)) != 0) && NRF_SPIM0->EVENTS_END == 1) {
    NRF_SPIM0->EVENTS_END = 0;
    spi.OnEndEvent();
  }

  if (((NRF_SPIM0->INTENSET & (1 << 19)) != 0) && NRF_SPIM0->EVENTS_STARTED == 1) {
    NRF_SPIM0->EVENTS_STARTED = 0;
    spi.OnStartedEvent();
  }

  if (((NRF_SPIM0->INTENSET & (1 << 1)) != 0) && NRF_SPIM0->EVENTS_STOPPED == 1) {
    NRF_SPIM0->EVENTS_STOPPED = 0;
  }
}
}

void RefreshWatchdog() {
  NRF_WDT->RR[0] = WDT_RR_RR_Reload;
}

uint8_t displayBuffer[displayWidth * bytesPerPixel];

void Process(void* /*instance*/) {
  RefreshWatchdog();
  APP_GPIOTE_INIT(2);

  NRF_LOG_INFO("Init...");
  spi.Init();
  spiNorFlash.Init();
  spiNorFlash.Wakeup();
  brightnessController.Init();
  lcd.Init();
  gfx.Init();

  NRF_LOG_INFO("Display logo")
  DisplayLogo();

  NRF_LOG_INFO("Erasing...");
  for (uint32_t erased = 0; erased < sizeof(recoveryImage); erased += 0x1000) {
    spiNorFlash.SectorErase(erased);
    RefreshWatchdog();
  }

  NRF_LOG_INFO("Writing factory image...");
  static constexpr uint32_t memoryChunkSize = 200;
  uint8_t writeBuffer[memoryChunkSize];
  for (size_t offset = 0; offset < sizeof(recoveryImage); offset += memoryChunkSize) {
    std::memcpy(writeBuffer, &recoveryImage[offset], memoryChunkSize);
    spiNorFlash.Write(offset, writeBuffer, memoryChunkSize);
    DisplayProgressBar((static_cast<float>(offset) / static_cast<float>(sizeof(recoveryImage))) * 100.0f, colorWhite);
    RefreshWatchdog();
  }
  NRF_LOG_INFO("Writing factory image done!");
  DisplayProgressBar(100.0f, colorGreen);

  while (1) {
    asm("nop");
  }
}

void DisplayLogo() {
  Pinetime::Tools::RleDecoder rleDecoder(infinitime_nb, sizeof(infinitime_nb));
  for (int i = 0; i < displayWidth; i++) {
    rleDecoder.DecodeNext(displayBuffer, displayWidth * bytesPerPixel);
    ulTaskNotifyTake(pdTRUE, 500);
    lcd.DrawBuffer(0, i, displayWidth, 1, reinterpret_cast<const uint8_t*>(displayBuffer), displayWidth * bytesPerPixel);
  }
}

void DisplayProgressBar(uint8_t percent, uint16_t color) {
  static constexpr uint8_t barHeight = 20;
  std::fill(displayBuffer, displayBuffer + (displayWidth * bytesPerPixel), color);
  for (int i = 0; i < barHeight; i++) {
    ulTaskNotifyTake(pdTRUE, 500);
    uint16_t barWidth = std::min(static_cast<float>(percent) * 2.4f, static_cast<float>(displayWidth));
    lcd.DrawBuffer(0, displayWidth - barHeight + i, barWidth, 1, reinterpret_cast<const uint8_t*>(displayBuffer), barWidth * bytesPerPixel);
  }
}

int mallocFailedCount = 0;
int stackOverflowCount = 0;
extern "C" {
void vApplicationMallocFailedHook() {
  mallocFailedCount++;
}

void vApplicationStackOverflowHook(TaskHandle_t /*xTask*/, char* /*pcTaskName*/) {
  stackOverflowCount++;
}
}

int main(void) {
  TaskHandle_t taskHandle;
  RefreshWatchdog();
  logger.Init();
  nrf_drv_clock_init();

  if (pdPASS != xTaskCreate(Process, "MAIN", 512, nullptr, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);

  vTaskStartScheduler();

  for (;;) {
    APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
  }
}
