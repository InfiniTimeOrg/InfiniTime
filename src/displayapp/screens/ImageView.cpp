#include <nrf_log.h>
#include "displayapp/screens/ImageView.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

ImageView::ImageView(DisplayApp* app, const char *path)
    : Screen(app)
{
  label = nullptr;

  lv_obj_t* image = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(image, path);
  lv_obj_align(image, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  const char *c = strrchr(path, '/') + 1;
  if (c == nullptr)
    c = path;

  strncpy(name, c, LFS_NAME_MAX-1);
  char *pchar = strchr(name, '_');
  while (pchar != nullptr) {
    *pchar = ' ';
    pchar = strchr(pchar + 1, '_');
  }

  ShowInfo();
}

ImageView::~ImageView() {
  lv_obj_clean(lv_scr_act());
}
