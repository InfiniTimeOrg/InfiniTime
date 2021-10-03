#include "LittleVgl.h"
#include "lv_pinetime_theme.h"

#include <FreeRTOS.h>
#include <task.h>
//#include <projdefs.h>
#include "drivers/Cst816s.h"
#include "drivers/St7789.h"

using namespace Pinetime::Components;

lv_style_t* LabelBigStyle = nullptr;

static void disp_flush(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p) {
  auto* lvgl = static_cast<LittleVgl*>(disp_drv->user_data);
  lvgl->FlushDisplay(area, color_p);
}

bool touchpad_read(lv_indev_drv_t* indev_drv, lv_indev_data_t* data) {
  auto* lvgl = static_cast<LittleVgl*>(indev_drv->user_data);
  return lvgl->GetTouchPadInfo(data);
}

LittleVgl::LittleVgl(Pinetime::Drivers::St7789& lcd, Pinetime::Drivers::Cst816S& touchPanel)
  : lcd {lcd}, touchPanel {touchPanel}, previousClick {0, 0} {

}

void LittleVgl::Init() {
  lv_init();
  InitTheme();
  InitDisplay();
  InitTouchpad();
}

void LittleVgl::InitDisplay() {
  lv_disp_buf_init(&disp_buf_2, buf2_1, buf2_2, LV_HOR_RES_MAX * 4); /*Initialize the display buffer*/
  lv_disp_drv_init(&disp_drv);                                       /*Basic initialization*/

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

void LittleVgl::InitTouchpad() {
  lv_indev_drv_t indev_drv;

  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  indev_drv.user_data = this;
  lv_indev_drv_register(&indev_drv);
}

void LittleVgl::SetFullRefresh(FullRefreshDirections direction) {
  if (scrollDirection == FullRefreshDirections::None) {
    scrollDirection = direction;
    if (scrollDirection == FullRefreshDirections::Down) {
      lv_disp_set_direction(lv_disp_get_default(), 1);
    } else if (scrollDirection == FullRefreshDirections::Right) {
      lv_disp_set_direction(lv_disp_get_default(), 2);
    } else if (scrollDirection == FullRefreshDirections::Left) {
      lv_disp_set_direction(lv_disp_get_default(), 3);
    } else if (scrollDirection == FullRefreshDirections::RightAnim) {
      lv_disp_set_direction(lv_disp_get_default(), 5);
    } else if (scrollDirection == FullRefreshDirections::LeftAnim) {
      lv_disp_set_direction(lv_disp_get_default(), 4);
    }
  }
}

void LittleVgl::FlushDisplay(const lv_area_t* area, lv_color_t* color_p) {
  uint16_t y1, y2, width, height = 0;

  ulTaskNotifyTake(pdTRUE, 200);
  // NOtification is still needed (even if there is a mutex on SPI) because of the DataCommand pin
  // which cannot be set/clear during a transfert.

  if ((scrollDirection == LittleVgl::FullRefreshDirections::Down) && (area->y2 == visibleNbLines - 1)) {
    writeOffset = ((writeOffset + totalNbLines) - visibleNbLines) % totalNbLines;
  } else if ((scrollDirection == FullRefreshDirections::Up) && (area->y1 == 0)) {
    writeOffset = (writeOffset + visibleNbLines) % totalNbLines;
  }

  y1 = (area->y1 + writeOffset) % totalNbLines;
  y2 = (area->y2 + writeOffset) % totalNbLines;

  width = (area->x2 - area->x1) + 1;
  height = (area->y2 - area->y1) + 1;

  if (scrollDirection == LittleVgl::FullRefreshDirections::Down) {

    if (area->y2 < visibleNbLines - 1) {
      uint16_t toScroll = 0;
      if (area->y1 == 0) {
        toScroll = height * 2;
        scrollDirection = FullRefreshDirections::None;
        lv_disp_set_direction(lv_disp_get_default(), 0);
      } else {
        toScroll = height;
      }

      if (scrollOffset >= toScroll)
        scrollOffset -= toScroll;
      else {
        toScroll -= scrollOffset;
        scrollOffset = (totalNbLines) -toScroll;
      }
      lcd.VerticalScrollStartAddress(scrollOffset);
    }

  } else if (scrollDirection == FullRefreshDirections::Up) {

    if (area->y1 > 0) {
      if (area->y2 == visibleNbLines - 1) {
        scrollOffset += (height * 2);
        scrollDirection = FullRefreshDirections::None;
        lv_disp_set_direction(lv_disp_get_default(), 0);
      } else {
        scrollOffset += height;
      }
      scrollOffset = scrollOffset % totalNbLines;
      lcd.VerticalScrollStartAddress(scrollOffset);
    }
  } else if (scrollDirection == FullRefreshDirections::Left or scrollDirection == FullRefreshDirections::LeftAnim) {
    if (area->x2 == visibleNbLines - 1) {
      scrollDirection = FullRefreshDirections::None;
      lv_disp_set_direction(lv_disp_get_default(), 0);
    }
  } else if (scrollDirection == FullRefreshDirections::Right or scrollDirection == FullRefreshDirections::RightAnim) {
    if (area->x1 == 0) {
      scrollDirection = FullRefreshDirections::None;
      lv_disp_set_direction(lv_disp_get_default(), 0);
    }
  }

  if (y2 < y1) {
    height = totalNbLines - y1;

    if (height > 0) {
      lcd.DrawBuffer(area->x1, y1, width, height, reinterpret_cast<const uint8_t*>(color_p), width * height * 2);
      ulTaskNotifyTake(pdTRUE, 100);
    }

    uint16_t pixOffset = width * height;
    height = y2 + 1;
    lcd.DrawBuffer(area->x1, 0, width, height, reinterpret_cast<const uint8_t*>(color_p + pixOffset), width * height * 2);

  } else {
    lcd.DrawBuffer(area->x1, y1, width, height, reinterpret_cast<const uint8_t*>(color_p), width * height * 2);
  }

  // IMPORTANT!!!
  // Inform the graphics library that you are ready with the flushing
  lv_disp_flush_ready(&disp_drv);
}

void LittleVgl::SetNewTouchPoint(uint16_t x, uint16_t y, bool contact) {
  tap_x = x;
  tap_y = y;
  tapped = contact;
}

bool LittleVgl::GetTouchPadInfo(lv_indev_data_t* ptr) {
  ptr->point.x = tap_x;
  ptr->point.y = tap_y;
  if (tapped) {
    ptr->state = LV_INDEV_STATE_PR;
  } else {
    ptr->state = LV_INDEV_STATE_REL;
  }
  return false;
}

void LittleVgl::InitTheme() {

  lv_theme_t* th = lv_pinetime_theme_init(
    LV_COLOR_WHITE, LV_COLOR_SILVER, 0, &jetbrains_mono_bold_20, &jetbrains_mono_bold_20, &jetbrains_mono_bold_20, &jetbrains_mono_bold_20);

  lv_theme_set_act(th);
}
