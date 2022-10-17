#include <nrf_log.h>
#include "displayapp/screens/ImageView.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

ImageView::ImageView(DisplayApp* app)
    : Screen(app)
{

}

ImageView::~ImageView() {
  lv_obj_clean(lv_scr_act());
}
