#include "InfiniPaint.h"
#include "../DisplayApp.h"
#include "../LittleVgl.h"

using namespace Pinetime::Applications::Screens;

InfiniPaint::InfiniPaint(Pinetime::Applications::DisplayApp* app, Pinetime::Components::LittleVgl& lvgl) : Screen(app), lvgl {lvgl} {
  std::fill(b, b + bufferSize, selectColor);
}

InfiniPaint::~InfiniPaint() {
  lv_obj_clean(lv_scr_act());
}

bool InfiniPaint::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case Pinetime::Applications::TouchEvents::LongTap:
      switch (color) {
        case 0:
          selectColor = lv_color_hex(0xFF00FF);
          break;
        case 1:
          selectColor = lv_palette_main(LV_PALETTE_GREEN);
          break;
        case 2:
          selectColor = lv_color_white();
          break;
        case 3:
          selectColor = lv_palette_main(LV_PALETTE_RED);
          break;
        case 4:
          selectColor = lv_palette_main(LV_PALETTE_CYAN);
          break;
        case 5:
          selectColor = lv_palette_main(LV_PALETTE_YELLOW);
          break;
        case 6:
          selectColor = lv_palette_main(LV_PALETTE_BLUE);
          break;
        case 7:
          selectColor = lv_color_black();
          break;

        default:
          color = 0;
          break;
      }

      std::fill(b, b + bufferSize, selectColor);
      color++;
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
