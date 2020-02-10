#include <FreeRTOS.h>
#include <projdefs.h>
#include <task.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <hal/nrf_rtc.h>
#include <libraries/log/nrf_log.h>

#include <libs/lvgl/src/lv_themes/lv_theme.h>
#include <libs/lvgl/src/lv_themes/lv_theme_night.h>

#include "LittleVgl.h"

using namespace Pinetime::Components;

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
  auto* lvgl = static_cast<LittleVgl*>(disp_drv->user_data);
  lvgl->FlushDisplay(area, color_p);
}

LittleVgl::LittleVgl(Pinetime::Drivers::St7789& lcd) : lcd{lcd} {
  lv_init();
  lv_theme_t* theme = lv_theme_night_init(10, NULL);
  lv_theme_set_current(theme);

  lv_disp_buf_init(&disp_buf_2, buf2_1, buf2_2, LV_HOR_RES_MAX * 2);   /*Initialize the display buffer*/
  lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

  /*Set up the functions to access to your display*/

  /*Set the resolution of the display*/
  disp_drv.hor_res = 240;
  disp_drv.ver_res = 240;

  /*Used to copy the buffer's content to the display*/
  disp_drv.flush_cb = disp_flush;
  /*Set a display buffer*/
  disp_drv.buffer = &disp_buf_2;
  disp_drv.user_data = this;

  /*Finally register the driver*/
  lv_disp_drv_register(&disp_drv);


}

void LittleVgl::FlushDisplay(const lv_area_t *area, lv_color_t *color_p) {
  auto x = area->x1;
  auto y = area->y1;
  auto width = (area->x2-area->x1)+1;
  auto height = (area->y2-area->y1)+1;
  lcd.BeginDrawBuffer(x, y, width, height);
  lcd.NextDrawBuffer(reinterpret_cast<const uint8_t *>(color_p), width * height*2) ;

  ulTaskNotifyTake(pdTRUE, 500);

  /* IMPORTANT!!!
   * Inform the graphics library that you are ready with the flushing*/
  lv_disp_flush_ready(&disp_drv);
}
