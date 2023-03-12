#include <nrf_log.h>
#include "displayapp/screens/Gallery.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

Gallery::Gallery(Pinetime::Controllers::FS& filesystem) : Screen(), filesystem(filesystem) {
  ListDir();

  if (nScreens == 0) {
    lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_static(title, "no images found");
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  } else {
    Open(0);
  }
}

Gallery::~Gallery() {
  lv_obj_clean(lv_scr_act());
}

bool Gallery::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case Pinetime::Applications::TouchEvents::SwipeRight:
      return Open(index - 1);
    case Pinetime::Applications::TouchEvents::SwipeLeft:
      return Open(index + 1);
    case Pinetime::Applications::TouchEvents::LongTap:
    case Pinetime::Applications::TouchEvents::DoubleTap:
      current->ToggleInfo();
      return true;
  }
  return false;
}

void Gallery::ListDir() {
  lfs_dir_t dir = {0};
  lfs_info info = {0};
  nScreens = 0;

  int res = filesystem.DirOpen(directory, &dir);
  if (res != 0) {
    NRF_LOG_INFO("[Gallery] can't find directory");
    return;
  }
  while (filesystem.DirRead(&dir, &info)) {
    if (info.type == LFS_TYPE_DIR)
      continue;
    nScreens++;
  }
  res = filesystem.DirClose(&dir);
  if (res != 0) {
    NRF_LOG_INFO("[Gallery] DirClose failed");
    return;
  }
}

bool Gallery::Open(int n) {
  if ((n < 0) || (n >= nScreens))
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
    if (info.type == LFS_TYPE_DIR)
      continue;
    if (n == i)
      break;
    i++;
  }
  res = filesystem.DirClose(&dir);
  if (res != 0) {
    NRF_LOG_INFO("[Gallery] DirClose failed");
    return false;
  }

  if (current != nullptr) {
    current.reset(nullptr);
  }

  char fullname[LFS_NAME_MAX] = "F:";
  strncat(fullname, directory, sizeof(fullname) - 2 - 1);
  strncat(fullname, info.name, sizeof(fullname) - strlen(directory) - 2 - 1);

  if (StringEndsWith(fullname, ".bin")) {
    current = std::make_unique<ImageView>(n, nScreens, fullname);
  } else if (StringEndsWith(fullname, ".txt")) {
    current = std::make_unique<TextView>(n, nScreens, fullname, filesystem);
  } else {
    return false;
  }

  return true;
}

int Gallery::StringEndsWith(const char* str, const char* suffix) {
  int str_len = strlen(str);
  int suffix_len = strlen(suffix);

  return (str_len >= suffix_len) && (0 == strcmp(str + (str_len - suffix_len), suffix));
}
