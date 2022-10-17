#include <nrf_log.h>
#include "displayapp/screens/ImageView.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

ImageView::ImageView(DisplayApp* app, Pinetime::Controllers::FS& filesystem)
    : Screen(app),
      filesystem(filesystem)
{
  listdir();

  if (nScreens == 0) {
    lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_static(title, "no images found");
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  }
}

ImageView::~ImageView() {
  lv_obj_clean(lv_scr_act());
}

void ImageView::listdir() {
  lfs_dir_t dir = {0};
  lfs_info info = {0};
  nScreens = 0;

  int res = filesystem.DirOpen(directory, &dir);
  if (res != 0) {
    NRF_LOG_INFO("[ImageView] can't find directory");
    return;
  }
  while (filesystem.DirRead(&dir, &info)) {
    nScreens++;
  }
  assert(filesystem.DirClose(&dir) == 0);
}
