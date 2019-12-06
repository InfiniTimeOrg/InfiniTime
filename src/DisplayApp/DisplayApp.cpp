#include "DisplayApp.h"
#include <FreeRTOS.h>
#include <task.h>
#include <libraries/log/nrf_log.h>
#include <boards.h>
#include <libraries/gfx/nrf_gfx.h>

using namespace Pinetime::Applications;

Pinetime::Drivers::st7789 lcd;
ret_code_t lcd_init() {
  return lcd.Init();
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

static const nrf_lcd_t nrf_lcd_st7789 = {
        .lcd_init = lcd_init,
        .lcd_uninit = lcd_dummy,
        .lcd_pixel_draw = lcd_pixel_draw,
        .lcd_rect_draw = lcd_rectangle_draw,
        .lcd_display = lcd_dummy,
        .lcd_rotation_set = lcd_rotation_set,
        .lcd_display_invert = lcd_display_invert,
        .p_lcd_cb = &st7789_cb
};

//extern const FONT_INFO orkney_24ptFontInfo;
//extern const uint_8 lCD_30ptBitmaps[];
extern const FONT_INFO lCD_70ptFontInfo;
//extern const FONT_CHAR_INFO lCD_30ptDescriptors[];

void DisplayApp::Start() {
  if (pdPASS != xTaskCreate(DisplayApp::Process, "DisplayApp", 256, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);

}

void DisplayApp::Process(void *instance) {
  auto* app = static_cast<DisplayApp*>(instance);

  NRF_LOG_INFO("DisplayApp task started!");
  app->gfx_initialization();
  uint8_t hour = 0;
  uint8_t minute = 1;
  while (1) {
    NRF_LOG_INFO("BlinkApp task running!");

    nrf_gfx_rect_t rect;
    rect.height = 74;
    rect.width = 52;
    rect.x = 7;
    rect.y = 78;
    nrf_gfx_rect_draw(&nrf_lcd_st7789, &rect, 2, 0x00000000, true);



    nrf_gfx_font_desc_t font;
    font.charInfo = lCD_70ptFontInfo.charInfo;
    font.data = lCD_70ptFontInfo.data;
    font.endChar = lCD_70ptFontInfo.endChar;
    font.height = lCD_70ptFontInfo.height;
    font.spacePixels = lCD_70ptFontInfo.spacePixels;
    font.startChar = lCD_70ptFontInfo.startChar;


    char t[2];
    sprintf(t, "%1d", hour);

    nrf_gfx_point_t point;
    point.x = 7;
    point.y = 78;
    nrf_gfx_print(&nrf_lcd_st7789,
                  &point,
                  0xffff,
                  t,
                  &font,
                  true);

//    point.x = 61;
//    point.y = 78;
//    nrf_gfx_print(&nrf_lcd_st7789,
//                  &point,
//                  0xffff,
//                  "2",
//                  &font,
//                  true);
//
//    point.x = 115;
//    point.y = 78;
//    nrf_gfx_print(&nrf_lcd_st7789,
//                  &point,
//                  0xffff,
//                  ":",
//                  &font,
//                  true);
//
//    point.x = 127;
//    point.y = 78;
//    nrf_gfx_print(&nrf_lcd_st7789,
//                  &point,
//                  0xffff,
//                  "3",
//                  &font,
//                  true);
//
//    point.x = 181;
//    point.y = 78;
//    nrf_gfx_print(&nrf_lcd_st7789,
//                  &point,
//                  0xffff,
//                  "4",
//                  &font,
//                  true);

    if(hour < 9)
      hour++;
    else hour = 0;
    vTaskDelay(1000);
  }
}

void DisplayApp::gfx_initialization(void)
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
  nrf_gfx_rect_draw(&nrf_lcd_st7789, &rect, 2, 0x00000000, true);

  nrf_gfx_point_t point;
  point.x = 7;
  point.y = 78;

  nrf_gfx_font_desc_t font;
  font.charInfo = lCD_70ptFontInfo.charInfo;
  font.data = lCD_70ptFontInfo.data;
  font.endChar = lCD_70ptFontInfo.endChar;
  font.height = lCD_70ptFontInfo.height;
  font.spacePixels = lCD_70ptFontInfo.spacePixels;
  font.startChar = lCD_70ptFontInfo.startChar;


  nrf_gfx_print(&nrf_lcd_st7789,
                &point,
                0xffff,
                "20:45",
                &font,
                true);

}
