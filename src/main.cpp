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
Pinetime::Drivers::st7789 lcd;

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

ret_code_t lcd_init() {
  lcd.Init();
}

void lcd_dummy() {

};

void lcd_pixel_draw(uint16_t x, uint16_t y, uint32_t color) {
  lcd.DrawPixel(x, y, color);
}


void lcd_rectangle_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  lcd.FillRectangle(x, y, width, height, color);
}

void lcd_rotation_set(nrf_lcd_rotation_t rotation) {

}

void lcd_display_invert(bool invert) {

}

static lcd_cb_t st7789_cb = {
        .height = 240,
        .width = 240
};

const nrf_lcd_t nrf_lcd_st7789 = {
        .lcd_init = lcd_init,
        .lcd_uninit = lcd_dummy,
        .lcd_pixel_draw = lcd_pixel_draw,
        .lcd_rect_draw = lcd_rectangle_draw,
        .lcd_display = lcd_dummy,
        .lcd_rotation_set = lcd_rotation_set,
        .lcd_display_invert = lcd_display_invert,
        .p_lcd_cb = &st7789_cb
};

extern const FONT_INFO orkney_24ptFontInfo;
static void gfx_initialization(void)
{
  nrf_gpio_cfg_output(14);
  nrf_gpio_cfg_output(22);
  nrf_gpio_cfg_output(23);
  nrf_gpio_pin_clear(14);
  nrf_gpio_pin_set(22);
  nrf_gpio_pin_set(23);

  APP_ERROR_CHECK(nrf_gfx_init(&nrf_lcd_st7789));
  nrf_gfx_rect_t rect;
  rect.height = 240;
  rect.width = 240;
  rect.x = 0;
  rect.y = 0;
  nrf_gfx_rect_draw(&nrf_lcd_st7789, &rect, 2, 0xaaaaaaaa, true);

  nrf_gfx_point_t point;
  point.x = 10;
  point.y = 10;

  nrf_gfx_font_desc_t font;
  font.charInfo = orkney_24ptFontInfo.charInfo;
  font.data = orkney_24ptFontInfo.data;
  font.endChar = orkney_24ptFontInfo.endChar;
  font.height = orkney_24ptFontInfo.height;
  font.spacePixels = orkney_24ptFontInfo.spacePixels;
  font.startChar = orkney_24ptFontInfo.startChar;


  nrf_gfx_print(&nrf_lcd_st7789,
                &point,
                0xffff,
                "#Pinetime\nRocks!",
                &font,
                true);

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

  gfx_initialization();
//  lcd.Init();
//  lcd.FillRectangle(0,0,240,240,0xffaa);
//  lcd.FillRectangle(10,10,50,50,0x011bb);
//
//  lcd.FillRectangle(120,120,120,120,0x1212);

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



