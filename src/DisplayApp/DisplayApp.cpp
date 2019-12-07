#include "DisplayApp.h"
#include <FreeRTOS.h>
#include <task.h>
#include <libraries/log/nrf_log.h>
#include <boards.h>
#include <nrf_font.h>
#include "Components/Gfx/Gfx.h"

using namespace Pinetime::Applications;

void DisplayApp::Start() {
  if (pdPASS != xTaskCreate(DisplayApp::Process, "DisplayApp", 256, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);

}

void DisplayApp::Process(void *instance) {
  auto* app = static_cast<DisplayApp*>(instance);
  NRF_LOG_INFO("DisplayApp task started!");
  app->InitHw();

  while (1) {
    NRF_LOG_INFO("BlinkApp task running!");
    vTaskDelay(1000);
  }
}

void DisplayApp::InitHw() {
  nrf_gpio_cfg_output(14);
  nrf_gpio_cfg_output(22);
  nrf_gpio_cfg_output(23);
  nrf_gpio_pin_clear(14);
  nrf_gpio_pin_clear(22);
  nrf_gpio_pin_clear(23);

  Drivers::SpiMaster::Parameters params;
  params.bitOrder = Drivers::SpiMaster::BitOrder::Msb_Lsb;
  params.mode = Drivers::SpiMaster::Modes::Mode3;
  params.Frequency = Drivers::SpiMaster::Frequencies::Freq8Mhz;
  params.pinCSN = 25;
  params.pinMISO = 4;
  params.pinMOSI = 3;
  params.pinSCK = 2;
  spi.Init(Drivers::SpiMaster::SpiModule::SPI0, params);

  lcd.reset(new Drivers::St7789(spi, 18));
  gfx.reset(new Components::Gfx(*lcd.get()));
  gfx->ClearScreen();

  uint8_t x = 7;
  gfx->DrawChar(&largeFont , '0', &x, 78, 0x0);

  x = 61;
  gfx->DrawChar(&largeFont, '1', &x, 78, 0x0);

  x = 115;
  gfx->DrawChar(&largeFont, ':', &x, 78, 0x0);

  x = 127;
  gfx->DrawChar(&largeFont, '2', &x, 78, 0x0);

  x = 181;
  gfx->DrawChar(&largeFont, '3', &x, 78, 0x0);
}
