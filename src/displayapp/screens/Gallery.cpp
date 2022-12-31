#include <nrf_log.h>
#include "displayapp/screens/Gallery.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

Gallery::Gallery(DisplayApp* app, Pinetime::Controllers::FS& filesystem) : Screen(app), filesystem(filesystem) {
  nScreens = CountFiles();

  if (nScreens == 0) {
    lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_static(title, "no images found");
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  } else {
    Open(0, DisplayApp::FullRefreshDirections::None);
  }
}

Gallery::~Gallery() {
  lv_obj_clean(lv_scr_act());
}

bool Gallery::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case Pinetime::Applications::TouchEvents::SwipeRight:
      return Open(index - 1, DisplayApp::FullRefreshDirections::Right);
    case Pinetime::Applications::TouchEvents::SwipeLeft:
      return Open(index + 1, DisplayApp::FullRefreshDirections::Left);
    case Pinetime::Applications::TouchEvents::LongTap:
    case Pinetime::Applications::TouchEvents::DoubleTap:
      current->ToggleInfo();
      return true;
  }
  return false;
}

uint8_t Gallery::CountFiles() {
  lfs_info info = {0};
  if (filesystem.Stat(index_file, &info) != LFS_ERR_OK) {
    NRF_LOG_INFO("[Gallery] can't stat index");
    return 0;
  }
  return info.size / LFS_NAME_MAX;
}

bool Gallery::Open(int n, DisplayApp::FullRefreshDirections direction) {
  if ((n < 0) || (n >= nScreens))
    return false;

  index = n;

  char fullname[LFS_NAME_MAX+2] = "F:";

  lfs_file_t fp;
  int res = filesystem.FileOpen(&fp, index_file, LFS_O_RDONLY);
  if (res != LFS_ERR_OK) {
    NRF_LOG_INFO("[Gallery] can't open index");
    return false;
  }
  res = filesystem.FileSeek(&fp, n * LFS_NAME_MAX);
  if (res != n * LFS_NAME_MAX) {
    filesystem.FileClose(&fp);
    NRF_LOG_INFO("[Gallery] can't seek index");
    return false;
  }
  res = filesystem.FileRead(&fp, reinterpret_cast<uint8_t*>(fullname)+2, LFS_NAME_MAX);
  if (res != LFS_NAME_MAX) {
    NRF_LOG_INFO("[Gallery] can't read index");
    filesystem.FileClose(&fp);
    return false;
  }
  filesystem.FileClose(&fp);

  if (current != nullptr) {
    current.reset(nullptr);
    app->SetFullRefresh(direction);
  }

  if (StringEndsWith(fullname, ".bin")) {
    current = std::make_unique<ImageView>(n, nScreens, app, fullname);
  } else if (StringEndsWith(fullname, ".txt")) {
    current = std::make_unique<TextView>(n, nScreens, app, fullname, filesystem);
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
