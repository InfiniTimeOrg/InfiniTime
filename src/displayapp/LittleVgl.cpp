#include "displayapp/LittleVgl.h"
#include "displayapp/InfiniTimeTheme.h"

#include <FreeRTOS.h>
#include <task.h>
#include "drivers/St7789.h"
#include "littlefs/lfs.h"
#include "components/fs/FS.h"

using namespace Pinetime::Components;

namespace {
  void InitTheme() {
    lv_theme_t* theme = lv_pinetime_theme_init();
    lv_theme_set_act(theme);
  }

  lv_fs_res_t lvglOpen(lv_fs_drv_t* drv, void* file_p, const char* path, lv_fs_mode_t /*mode*/) {
    lfs_file_t* file = static_cast<lfs_file_t*>(file_p);
    Pinetime::Controllers::FS* filesys = static_cast<Pinetime::Controllers::FS*>(drv->user_data);
    int res = filesys->FileOpen(file, path, LFS_O_RDONLY);
    if (res == 0) {
      if (file->type == 0) {
        return LV_FS_RES_FS_ERR;
      } else {
        return LV_FS_RES_OK;
      }
    }
    return LV_FS_RES_NOT_EX;
  }

  lv_fs_res_t lvglClose(lv_fs_drv_t* drv, void* file_p) {
    Pinetime::Controllers::FS* filesys = static_cast<Pinetime::Controllers::FS*>(drv->user_data);
    lfs_file_t* file = static_cast<lfs_file_t*>(file_p);
    filesys->FileClose(file);

    return LV_FS_RES_OK;
  }

  lv_fs_res_t lvglRead(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br) {
    Pinetime::Controllers::FS* filesys = static_cast<Pinetime::Controllers::FS*>(drv->user_data);
    lfs_file_t* file = static_cast<lfs_file_t*>(file_p);
    filesys->FileRead(file, static_cast<uint8_t*>(buf), btr);
    *br = btr;
    return LV_FS_RES_OK;
  }

  lv_fs_res_t lvglSeek(lv_fs_drv_t* drv, void* file_p, uint32_t pos) {
    Pinetime::Controllers::FS* filesys = static_cast<Pinetime::Controllers::FS*>(drv->user_data);
    lfs_file_t* file = static_cast<lfs_file_t*>(file_p);
    filesys->FileSeek(file, pos);
    return LV_FS_RES_OK;
  }
}

static void disp_flush(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p) {
  auto* lvgl = static_cast<LittleVgl*>(disp_drv->user_data);
  lvgl->FlushDisplay(area, color_p);
}

static void rounder(lv_disp_drv_t* disp_drv, lv_area_t* area) {
  auto* lvgl = static_cast<LittleVgl*>(disp_drv->user_data);
  if (lvgl->GetFullRefresh()) {
    area->x1 = 0;
    area->x2 = LV_HOR_RES - 1;
    area->y1 = 0;
    area->y2 = LV_VER_RES - 1;
  }
}

bool touchpad_read(lv_indev_drv_t* indev_drv, lv_indev_data_t* data) {
  auto* lvgl = static_cast<LittleVgl*>(indev_drv->user_data);
  return lvgl->GetTouchPadInfo(data);
}

LittleVgl::LittleVgl(Pinetime::Drivers::St7789& lcd, Pinetime::Controllers::FS& filesystem) : lcd {lcd}, filesystem {filesystem} {
}

void LittleVgl::Init() {
  lv_init();
  InitTheme();
  InitDisplay();
  InitTouchpad();
  InitFileSystem();
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
  disp_drv.rounder_cb = rounder;

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

void LittleVgl::InitFileSystem() {
  lv_fs_drv_t fs_drv;
  lv_fs_drv_init(&fs_drv);

  fs_drv.file_size = sizeof(lfs_file_t);
  fs_drv.letter = 'F';
  fs_drv.open_cb = lvglOpen;
  fs_drv.close_cb = lvglClose;
  fs_drv.read_cb = lvglRead;
  fs_drv.seek_cb = lvglSeek;

  fs_drv.user_data = &filesystem;

  lv_fs_drv_register(&fs_drv);
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
  fullRefresh = true;
}

void LittleVgl::FlushDisplay(const lv_area_t* area, lv_color_t* color_p) {
  uint16_t y1, y2, width, height = 0;

  ulTaskNotifyTake(pdTRUE, 200);
  // Notification is still needed (even if there is a mutex on SPI) because of the DataCommand pin
  // which cannot be set/clear during a transfer.

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

void LittleVgl::SetNewTouchPoint(int16_t x, int16_t y, bool contact) {
  if (contact) {
    if (!isCancelled) {
      touchPoint = {x, y};
      tapped = true;
    }
  } else {
    if (isCancelled) {
      touchPoint = {-1, -1};
      tapped = false;
      isCancelled = false;
    } else {
      touchPoint = {x, y};
      tapped = false;
    }
  }
}

void LittleVgl::CancelTap() {
  if (tapped) {
    isCancelled = true;
    touchPoint = {-1, -1};
  }
}

bool LittleVgl::GetTouchPadInfo(lv_indev_data_t* ptr) {
  ptr->point.x = touchPoint.x;
  ptr->point.y = touchPoint.y;
  if (tapped) {
    ptr->state = LV_INDEV_STATE_PR;
  } else {
    ptr->state = LV_INDEV_STATE_REL;
  }
  return false;
}
