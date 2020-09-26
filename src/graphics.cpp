#include <legacy/nrf_drv_clock.h>
#include <softdevice/common/nrf_sdh.h>
#include <drivers/SpiMaster.h>
#include <drivers/Spi.h>
#include <drivers/SpiNorFlash.h>
#include <libraries/log/nrf_log.h>
#include "bootloader/boot_graphics.h"
#include <FreeRTOS.h>
#include <task.h>
#include <legacy/nrf_drv_gpiote.h>
#include <libraries/gpiote/app_gpiote.h>
#include <hal/nrf_wdt.h>
#include <cstring>
#include <Components/Gfx/Gfx.h>
#include <drivers/St7789.h>
#include <Components/Brightness/BrightnessController.h>

#if NRF_LOG_ENABLED
#include "Logging/NrfLogger.h"
Pinetime::Logging::NrfLogger logger;
#else
#include "Logging/DummyLogger.h"
Pinetime::Logging::DummyLogger logger;
#endif

static constexpr uint8_t pinSpiSck = 2;
static constexpr uint8_t pinSpiMosi = 3;
static constexpr uint8_t pinSpiMiso = 4;
static constexpr uint8_t pinSpiFlashCsn = 5;
static constexpr uint8_t pinLcdCsn = 25;
static constexpr uint8_t pinLcdDataCommand = 18;

Pinetime::Drivers::SpiMaster spi{Pinetime::Drivers::SpiMaster::SpiModule::SPI0, {
        Pinetime::Drivers::SpiMaster::BitOrder::Msb_Lsb,
        Pinetime::Drivers::SpiMaster::Modes::Mode3,
        Pinetime::Drivers::SpiMaster::Frequencies::Freq8Mhz,
        pinSpiSck,
        pinSpiMosi,
        pinSpiMiso
  }
};
Pinetime::Drivers::Spi flashSpi{spi, pinSpiFlashCsn};
Pinetime::Drivers::SpiNorFlash spiNorFlash{flashSpi};

Pinetime::Drivers::Spi lcdSpi {spi, pinLcdCsn};
Pinetime::Drivers::St7789 lcd {lcdSpi, pinLcdDataCommand};

Pinetime::Components::Gfx gfx{lcd};
Pinetime::Controllers::BrightnessController brightnessController;

extern "C" {
void vApplicationIdleHook(void) {

}

void SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler(void) {
  if(((NRF_SPIM0->INTENSET & (1<<6)) != 0) && NRF_SPIM0->EVENTS_END == 1) {
    NRF_SPIM0->EVENTS_END = 0;
    spi.OnEndEvent();
  }

  if(((NRF_SPIM0->INTENSET & (1<<19)) != 0) && NRF_SPIM0->EVENTS_STARTED == 1) {
    NRF_SPIM0->EVENTS_STARTED = 0;
    spi.OnStartedEvent();
  }

  if(((NRF_SPIM0->INTENSET & (1<<1)) != 0) && NRF_SPIM0->EVENTS_STOPPED == 1) {
    NRF_SPIM0->EVENTS_STOPPED = 0;
  }
}
}

void Process(void* instance) {
  // Wait before erasing the memory to let the time to the SWD debugger to flash a new firmware before running this one.
  vTaskDelay(5000);

  APP_GPIOTE_INIT(2);

  NRF_LOG_INFO("Init...");
  spi.Init();
  spiNorFlash.Init();
  spiNorFlash.Wakeup();
  brightnessController.Init();
  lcd.Init();
  gfx.Init();
  NRF_LOG_INFO("Init Done!")

  NRF_LOG_INFO("Erasing...");
  for (uint32_t erased = 0; erased < graphicSize; erased += 0x1000) {
    spiNorFlash.SectorErase(erased);
  }
  NRF_LOG_INFO("Erase done!");

  NRF_LOG_INFO("Writing graphic...");
  static constexpr uint32_t memoryChunkSize = 200;
  uint8_t writeBuffer[memoryChunkSize];
  for(int offset = 0; offset < 115200; offset+=memoryChunkSize) {
    std::memcpy(writeBuffer, &graphicBuffer[offset], memoryChunkSize);
    spiNorFlash.Write(offset, writeBuffer, memoryChunkSize);
  }
  NRF_LOG_INFO("Writing graphic done!");

  NRF_LOG_INFO("Read memory and display the graphic...");
  static constexpr uint32_t screenWidth = 240;
  static constexpr uint32_t screenWidthInBytes = screenWidth*2; // LCD display 16bits color (1 pixel = 2 bytes)
  uint16_t displayLineBuffer[screenWidth];
  for(uint32_t line = 0; line < screenWidth; line++) {
    spiNorFlash.Read(line*screenWidthInBytes, reinterpret_cast<uint8_t *>(displayLineBuffer), screenWidth);
    spiNorFlash.Read((line*screenWidthInBytes)+screenWidth, reinterpret_cast<uint8_t *>(displayLineBuffer) + screenWidth, screenWidth);
    for(uint32_t col = 0; col < screenWidth; col++) {
      gfx.pixel_draw(col, line, displayLineBuffer[col]);
    }
  }

  NRF_LOG_INFO("Done!");

  while(1) {
    asm("nop" );
  }
}

int main(void) {
  TaskHandle_t taskHandle;

  logger.Init();
  nrf_drv_clock_init();

  if (pdPASS != xTaskCreate(Process, "MAIN", 512, nullptr, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);

  vTaskStartScheduler();

  for (;;) {
    APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
  }
}
