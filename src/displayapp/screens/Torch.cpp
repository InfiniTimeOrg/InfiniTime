#include "Torch.h"
#include <lvgl/lvgl.h>
#include "../LittleVgl.h"

using namespace Pinetime::Applications::Screens;

Torch::Torch(Pinetime::Applications::DisplayApp *app, Controllers::BrightnessController& brightness, Pinetime::Components::LittleVgl& lvgl) : Screen(app), brightness{brightness}, lvgl{lvgl} {

  // Save previous brightness
  brightness.Backup();

  // Set max brightness
  brightness.Set(Controllers::BrightnessController::Levels::High);

  // create a style for the background (white):
  static lv_style_t WhiteScreen;
  lv_style_init(&WhiteScreen);
  lv_style_set_bg_color(&WhiteScreen, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  // create the white background using lv_obj):
  BgScreen = lv_obj_create(lv_scr_act(), NULL);
  lv_obj_set_size(BgScreen, 240, 240);  // set dimensions
  lv_obj_align(BgScreen, NULL, LV_ALIGN_CENTER, 0, 0); // set the label in the center
  lv_obj_add_style(BgScreen, LV_OBJ_PART_MAIN, &WhiteScreen); // add the style to the background object
  lv_obj_move_foreground(BgScreen); // bring the object to foreground
}

Torch::~Torch() {
  lv_obj_clean(lv_scr_act());
}

bool Torch::OnButtonPushed() {
  brightness.Restore();
  running = false;
  return true;
}

bool Torch::Refresh() {
  return running;
}
