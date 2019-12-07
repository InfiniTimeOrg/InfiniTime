#include "DisplayApp.h"
#include <FreeRTOS.h>
#include <task.h>
#include <libraries/log/nrf_log.h>
#include <boards.h>
#include <nrf_font.h>
#include <hal/nrf_rtc.h>
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

    app->Refresh();

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
  gfx->DrawChar(&largeFont , '0', &x, 78, 0xffff);

  x = 61;
  gfx->DrawChar(&largeFont, '0', &x, 78, 0xffff);

  x = 94;
  gfx->DrawChar(&largeFont, ':', &x, 78, 0xffff);

  x = 127;
  gfx->DrawChar(&largeFont, '0', &x, 78, 0xffff);

  x = 181;
  gfx->DrawChar(&largeFont, '0', &x, 78, 0xffff);
}

void DisplayApp::Refresh() {
  uint32_t systick_counter = nrf_rtc_counter_get(portNRF_RTC_REG);
  auto raw = systick_counter / 1000;

  // TODO make this better!
  minutes = raw / 60;
  seconds = raw - (minutes*60);

  char secondChar[3];
  sprintf(secondChar, "%02d", seconds);

  char minutesChar[3];
  sprintf(minutesChar, "%02d", minutes);

  uint8_t x = 7;
  if(minutesChar[0] != currentChar[0]) {
    gfx->DrawChar(&largeFont, minutesChar[0], &x, 78, 0xffff);
    currentChar[0] = minutesChar[0];
  }

  x = 61;
  if(minutesChar[1] != currentChar[1]) {
    gfx->DrawChar(&largeFont, minutesChar[1], &x, 78, 0xffff);
    currentChar[1] = minutesChar[1];
  }

  x = 127;
  if(secondChar[0] != currentChar[2]) {
    gfx->DrawChar(&largeFont, secondChar[0], &x, 78, 0xffff);
    currentChar[2] = secondChar[0];
  }

  x = 181;
  if(secondChar[1] != currentChar[3]) {
    gfx->DrawChar(&largeFont, secondChar[1], &x, 78, 0xffff);
    currentChar[3] = secondChar[1];
  }

}
