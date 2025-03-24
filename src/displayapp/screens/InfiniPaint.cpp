#include "displayapp/screens/InfiniPaint.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/LittleVgl.h"
#include "displayapp/InfiniTimeTheme.h"

#include <algorithm> // std::fill

using namespace Pinetime::Applications::Screens;

InfiniPaint::InfiniPaint(Pinetime::Components::LittleVgl& lvgl, Pinetime::Controllers::MotorController& motor)
  : lvgl {lvgl}, motor {motor} {
  std::fill(b, b + bufferSize, selectColor);
}

InfiniPaint::~InfiniPaint() {
  lv_obj_clean(lv_scr_act());
}

bool InfiniPaint::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case Pinetime::Applications::TouchEvents::LongTap:
      color = (color + 1) % 8;
      switch (color) {
        case 0:
          selectColor = LV_COLOR_MAGENTA;
          break;
        case 1:
          selectColor = Colors::green;
          break;
        case 2:
          selectColor = LV_COLOR_WHITE;
          break;
        case 3:
          selectColor = LV_COLOR_RED;
          break;
        case 4:
          selectColor = LV_COLOR_CYAN;
          break;
        case 5:
          selectColor = LV_COLOR_YELLOW;
          break;
        case 6:
          selectColor = LV_COLOR_BLUE;
          break;
        case 7:
          selectColor = LV_COLOR_BLACK;
          break;

        default:
          color = 0;
          break;
      }

      std::fill(b, b + bufferSize, selectColor);
      motor.RunForDuration(35);
      return true;
    default:
      return true;
  }
  return true;
}

bool InfiniPaint::OnTouchEvent(uint16_t x, uint16_t y) {
  lv_area_t area;
  area.x1 = x - (width / 2);
  area.y1 = y - (height / 2);
  area.x2 = x + (width / 2) - 1;
  area.y2 = y + (height / 2) - 1;
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  lvgl.FlushDisplay(&area, b);
  return true;
}
