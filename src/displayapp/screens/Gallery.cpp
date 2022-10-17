#include <nrf_log.h>
#include "displayapp/screens/Gallery.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

Gallery::Gallery(DisplayApp* app, Pinetime::Controllers::FS& filesystem)
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

Gallery::~Gallery() {
  lv_obj_clean(lv_scr_act());
}

void Gallery::listdir() {
  lfs_dir_t dir = {0};
  lfs_info info = {0};
  nScreens = 0;

  int res = filesystem.DirOpen(directory, &dir);
  if (res != 0) {
    NRF_LOG_INFO("[Gallery] can't find directory");
    return;
  }
  while (filesystem.DirRead(&dir, &info)) {
    if(info.type == LFS_TYPE_DIR)
      continue;
    nScreens++;
  }
  assert(filesystem.DirClose(&dir) == 0);
}

bool Gallery::open(int n, DisplayApp::FullRefreshDirections direction) {
  if ( (n < 0) || (n >= nScreens) )
    return false;

  lfs_dir_t dir = {0};
  lfs_info info = {0};

  int res = filesystem.DirOpen(directory, &dir);
  if (res != 0) {
    NRF_LOG_INFO("[Gallery] can't find directory");
    return false;
  }
  while (filesystem.DirRead(&dir, &info) && n > 0) {
    if(info.type == LFS_TYPE_DIR)
      continue;
    n--;
  }
  assert(filesystem.DirClose(&dir) == 0);

  lfs_file f = {0};
  if (filesystem.FileOpen(&f, info.name, LFS_O_RDONLY) != LFS_ERR_OK)
    return false;

  assert(filesystem.FileClose(&f) == 0);

  return true;
}
