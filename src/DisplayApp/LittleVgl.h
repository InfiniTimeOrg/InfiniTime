#pragma once

#include <libs/lvgl/src/lv_hal/lv_hal.h>
#include <drivers/St7789.h>


static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

namespace Pinetime {
  namespace Components {
    class LittleVgl {
      public:
        LittleVgl(Pinetime::Drivers::St7789& lcd);
        void FlushDisplay(const lv_area_t * area, lv_color_t * color_p);


      private:
        Pinetime::Drivers::St7789& lcd;

        lv_disp_buf_t disp_buf_2;
        lv_color_t buf2_1[LV_HOR_RES_MAX * 2];
        lv_color_t buf2_2[LV_HOR_RES_MAX * 2];

        lv_disp_drv_t disp_drv;

    };
  }
}

