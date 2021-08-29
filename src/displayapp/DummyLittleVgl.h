#pragma once

#include <libs/lvgl/src/lv_core/lv_style.h>
#include <libs/lvgl/src/lv_themes/lv_theme.h>
#include <libs/lvgl/src/lv_hal/lv_hal.h>
#include <drivers/St7789.h>
#include <drivers/Cst816s.h>

namespace Pinetime {
  namespace Components {
    class LittleVgl {
    public:
      enum class FullRefreshDirections { None, Up, Down };
      LittleVgl(Pinetime::Drivers::St7789& lcd, Pinetime::Drivers::Cst816S& touchPanel) {
      }

      LittleVgl(const LittleVgl&) = delete;
      LittleVgl& operator=(const LittleVgl&) = delete;
      LittleVgl(LittleVgl&&) = delete;
      LittleVgl& operator=(LittleVgl&&) = delete;

      void Init() {

      }

      void FlushDisplay(const lv_area_t* area, lv_color_t* color_p) {
      }
      bool GetTouchPadInfo(lv_indev_data_t* ptr) {
        return false;
      }
      void SetFullRefresh(FullRefreshDirections direction) {
      }
      void SetNewTapEvent(uint16_t x, uint16_t y) {
      }
      void SetNewTouchPoint(uint16_t x, uint16_t y, bool contact) {

      }
    };
  }
}
