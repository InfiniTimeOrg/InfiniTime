#include <nrf_log.h>
#include "displayapp/screens/ImageView.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

ImageView::ImageView(DisplayApp* app, const char *path)
    : Screen(app)
{
  lv_obj_t* image = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(image, path);
  lv_obj_align(image, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
}

ImageView::~ImageView() {
  lv_obj_clean(lv_scr_act());
}
