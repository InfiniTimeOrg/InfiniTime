#include <libs/lvgl/lvgl.h>
#include <libraries/log/nrf_log.h>
#include "InfiniPaint.h"
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;
extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;

InfiniPaint::InfiniPaint(Pinetime::Applications::DisplayApp *app,
                         Pinetime::Components::LittleVgl& lvgl,
                         Controllers::BleMouse& bleMouse) :
                         Screen(app), lvgl{lvgl}, bleMouse{bleMouse} {
  app->SetTouchMode(DisplayApp::TouchModes::Polling);
  std::fill(b, b+bufferSize, LV_COLOR_WHITE);
}

InfiniPaint::~InfiniPaint() {
  // Reset the touchmode
  app->SetTouchMode(DisplayApp::TouchModes::Gestures);
  lv_obj_clean(lv_scr_act());
}

bool InfiniPaint::Refresh() {
  timeout--;
  if(timeout == 0) {
    moving = false;
  }
  return running;
}

bool InfiniPaint::OnButtonPushed() {
  running = false;
  return true;
}

bool InfiniPaint::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  NRF_LOG_INFO("TOUCH %d", event);
  return true;
}

bool InfiniPaint::OnTouchEvent(uint16_t x, uint16_t y) {
  NRF_LOG_INFO("TOUCH %d, %d", x, y);

  lv_area_t area;
  area.x1 = x-(width/2);
  area.y1 = y-(height/2);
  area.x2 = x+(width/2)-1;
  area.y2 = y+(height/2)-1;
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  lvgl.FlushDisplay(&area, b);

  if(!moving) {
    moving = true;
    lastX = x;
    lastY = y;

  } else {

    bleMouse.Move((x - lastX)*2, (y - lastY)*2);
    lastX = x;
    lastY = y;
  }
  timeout = 20;

  return true;
}

