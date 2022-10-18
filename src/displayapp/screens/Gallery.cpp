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
  } else {
    open(0, DisplayApp::FullRefreshDirections::None);
  }
}

Gallery::~Gallery() {
  lv_obj_clean(lv_scr_act());
}

bool Gallery::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case Pinetime::Applications::TouchEvents::SwipeRight:
      return open(index - 1, DisplayApp::FullRefreshDirections::Right);
    case Pinetime::Applications::TouchEvents::SwipeLeft:
      return open(index + 1, DisplayApp::FullRefreshDirections::Left);
    case Pinetime::Applications::TouchEvents::LongTap:
    case Pinetime::Applications::TouchEvents::DoubleTap:
      current->ToggleInfo();
      return true;
  }
  return false;
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

  index = n;

  lfs_dir_t dir = {0};
  lfs_info info = {0};

  int res = filesystem.DirOpen(directory, &dir);
  if (res != 0) {
    NRF_LOG_INFO("[Gallery] can't find directory");
    return false;
  }
  int i = 0;
  while (filesystem.DirRead(&dir, &info)) {
    if(info.type == LFS_TYPE_DIR)
      continue;
    if (n == i) break;
    i++;
  }
  assert(filesystem.DirClose(&dir) == 0);

  if (current != nullptr) {
    current.reset(nullptr);
    app->SetFullRefresh(direction);
  }

  char fullname[LFS_NAME_MAX] = "F:";
  strncat(fullname, directory, sizeof(fullname) - 2 - 1);
  strncat(fullname, info.name,
             sizeof(fullname) - strlen(directory) - 2 - 1);
  current = std::make_unique<ImageView>(n, nScreens, app, fullname);

  return true;
}
