#include <FreeRTOS.h>
#include <task.h>
#include <libraries/log/nrf_log.h>
#include <BlinkApp/BlinkApp.h>
#include <boards.h>
#include <libraries/bsp/bsp.h>
#include <legacy/nrf_drv_clock.h>
#include <libraries/timer/app_timer.h>
#include <libraries/gpiote/app_gpiote.h>
#include <libraries/gfx/nrf_lcd.h>
#include <drivers/st7789.h>
#include <DisplayApp/DisplayApp.h>
#include "nrf_gfx.h"


#if NRF_LOG_ENABLED
#include "Logging/NrfLogger.h"
Pinetime::Logging::NrfLogger logger;
#else
#include "Logging/DummyLogger.h"
Pinetime::Logging::DummyLogger logger;
#endif

Pinetime::Applications::BlinkApp blinkApp;
Pinetime::Applications::DisplayApp displayApp;
TaskHandle_t systemThread;
//Pinetime::Drivers::st7789 lcd;

extern "C" {
  void vApplicationIdleHook() {
    logger.Resume();
  }

  void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName ) {
    bsp_board_led_on(3);
  }
}

static void bsp_event_handler(bsp_event_t event)
{
  switch (event)
  {
    case BSP_EVENT_KEY_0:
      NRF_LOG_INFO("Button pressed");
      break;
    default:
      break;
  }
}





void SystemTask(void *) {
  APP_GPIOTE_INIT(2);
  app_timer_init();

  nrf_gpio_cfg_output(14);
  nrf_gpio_cfg_output(22);
  nrf_gpio_cfg_output(23);
  nrf_gpio_pin_clear(14);
  nrf_gpio_pin_clear(22);
  nrf_gpio_pin_clear(23);
//  lcd.Init();
//  lcd.FillRectangle(0,0,240,240,0xffaa);
//  lcd.FillRectangle(10,10,50,50,0x011bb);
//
//  lcd.FillRectangle(120,120,120,120,0x1212);

  blinkApp.Start();
  displayApp.Start();

  while (1) {
    vTaskSuspend(nullptr);
  }
}



int main(void) {
  logger.Init();
  nrf_drv_clock_init();

  if (pdPASS != xTaskCreate(SystemTask, "MAIN", 256, nullptr, 0, &systemThread))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);

  vTaskStartScheduler();

  for (;;) {
    APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
  }
}



