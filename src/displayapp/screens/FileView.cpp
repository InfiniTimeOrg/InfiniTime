#include <nrf_log.h>
#include "displayapp/screens/FileView.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

FileView::FileView(uint8_t screenID, uint8_t nScreens, DisplayApp* app, const char *path)
    : screenID(screenID),
      nScreens(nScreens),
      Screen(app)
{
  label = nullptr;
  pageIndicatorBase = nullptr;
  pageIndicator = nullptr;

  const char *c = strrchr(path, '/') + 1;
  if (c == nullptr)
    c = path;

  strncpy(name, c, LFS_NAME_MAX-1);
  char *pchar = strchr(name, '_');
  while (pchar != nullptr) {
    *pchar = ' ';
    pchar = strchr(pchar + 1, '_');
  }

  pageIndicatorBasePoints[0].x = 0;
  pageIndicatorBasePoints[0].y = 1;
  pageIndicatorBasePoints[1].x = LV_HOR_RES - 1;
  pageIndicatorBasePoints[1].y = 1;

  const int16_t indicatorSize = LV_HOR_RES / nScreens;
  const int16_t indicatorPos = indicatorSize * screenID;

  pageIndicatorPoints[0].x = indicatorPos;
  pageIndicatorPoints[0].y = 1;
  pageIndicatorPoints[1].x = indicatorPos + indicatorSize;
  pageIndicatorPoints[1].y = 1;

  ShowInfo();
}

void FileView::ShowInfo() {
  if(label != nullptr) {
    return;
  }
  label = lv_btn_create(lv_scr_act(), nullptr);
  label->user_data = this;

  lv_obj_set_height(label, 20);
  lv_obj_set_width(label, LV_HOR_RES);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  lv_obj_t *txtMessage = lv_label_create(label, nullptr);
  lv_obj_set_style_local_bg_color(label, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_NAVY);
  lv_label_set_text_static(txtMessage, name);

  pageIndicatorBase = lv_line_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_line_width(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_line_color(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, Colors::bgDark);
  lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);

  pageIndicator = lv_line_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_line_width(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_line_color(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_line_set_points(pageIndicator, pageIndicatorPoints, 2);
}

void FileView::HideInfo() {
  lv_obj_del(label);
  lv_obj_del(pageIndicatorBase);
  lv_obj_del(pageIndicator);

  label = nullptr;
  pageIndicatorBase = nullptr;
  pageIndicator = nullptr;
}

void FileView::ToggleInfo() {
  if (label == nullptr)
    ShowInfo();
  else
    HideInfo();
}

FileView::~FileView() {
  lv_obj_clean(lv_scr_act());
}

ImageView::ImageView(uint8_t screenID, uint8_t nScreens, DisplayApp* app, const char *path)
    : FileView(screenID, nScreens, app, path) {
  lv_obj_t* image = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(image, path);
  lv_obj_align(image, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
}

TextView::TextView(uint8_t screenID, uint8_t nScreens, DisplayApp* app, const char *path, Pinetime::Controllers::FS& fs)
    : FileView(screenID, nScreens, app, path) {

  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(label, LV_HOR_RES);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  lfs_info info = {0};
  if (fs.Stat(path+2, &info) != LFS_ERR_OK && info.type != LFS_TYPE_DIR) {
    lv_label_set_text_static(label, "could not open file");
    return;
  }

  char *buf =  (char *)lv_mem_alloc(info.size);
  if (buf == nullptr) {
    lv_label_set_text_static(label, "could not open file");
    return;
  }

  lfs_file_t fp;
  if (fs.FileOpen(&fp, path+2, LFS_O_RDONLY) != LFS_ERR_OK) {
    lv_label_set_text_static(label, "could not open file");
    lv_mem_free(buf);
    return;
  }

  fs.FileRead(&fp, reinterpret_cast<uint8_t*>(buf), info.size);
  lv_label_set_text(label, buf);
  lv_mem_free(buf);

  fs.FileClose(&fp);
}
