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
#include "nrf_gfx.h"


#if NRF_LOG_ENABLED
#include "Logging/NrfLogger.h"
Pinetime::Logging::NrfLogger logger;
#else
#include "Logging/DummyLogger.h"
Pinetime::Logging::DummyLogger logger;
#endif

Pinetime::Applications::BlinkApp blinkApp;
TaskHandle_t systemThread;


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

extern const nrf_lcd_t nrf_lcd_st7735;
static nrf_lcd_t const * p_lcd = &nrf_lcd_st7735;
static void gfx_initialization(void)
{
  nrf_gpio_cfg_output(14);
  nrf_gpio_cfg_output(22);
  nrf_gpio_cfg_output(23);
  nrf_gpio_pin_clear(14);
  nrf_gpio_pin_set(22);
  nrf_gpio_pin_set(23);

  APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
  nrf_gfx_rect_t rect;
  rect.height = 10;
  rect.width = 10;
  rect.x = 10;
  rect.y = 10;
  nrf_gfx_rect_draw(p_lcd, &rect, 2, 0xffffffff, true);
}

void SystemTask(void *) {
  APP_GPIOTE_INIT(2);
  app_timer_init();

  bsp_board_init(BSP_INIT_LEDS|BSP_INIT_BUTTONS);
  bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);

  gfx_initialization();


  blinkApp.Start();

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



