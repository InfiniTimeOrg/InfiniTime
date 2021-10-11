#include "SettingSteps.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_event_t *event) {
    SettingSteps* screen = static_cast<SettingSteps *>(lv_event_get_user_data(event));
    screen->UpdateSelected(lv_event_get_target(event), event);
  }
}

SettingSteps::SettingSteps(
  Pinetime::Applications::DisplayApp *app, Pinetime::Controllers::Settings &settingsController) :
  Screen(app),
  settingsController{settingsController}
{

  lv_obj_t * container1 = lv_obj_create(lv_scr_act());

  lv_obj_set_style_bg_opa(container1, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_all(container1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_row(container1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_column(container1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(container1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_pos(container1, 30, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 50);
  lv_obj_set_height(container1, LV_VER_RES - 60);
  lv_obj_set_layout(container1, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(container1, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(container1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

  lv_obj_t * title = lv_label_create(lv_scr_act());
  lv_label_set_text_static(title,"Daily steps goal");
  lv_obj_set_style_text_align(title, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 15, 15);

  lv_obj_t * icon = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(icon, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  
  lv_label_set_text_static(icon, Symbols::shoe);
  lv_obj_set_style_text_align(icon, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align_to(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);


  stepValue = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(stepValue, &jetbrains_mono_42, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_fmt(stepValue, "%lu", settingsController.GetStepsGoal());
  lv_obj_set_style_text_align(stepValue, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(stepValue, LV_ALIGN_CENTER, 0, -10);

  btnPlus = lv_btn_create(lv_scr_act());
  btnPlus->user_data = this;
  lv_obj_set_size(btnPlus, 80, 50);
  lv_obj_align(btnPlus, LV_ALIGN_CENTER, 55, 80);
  txtBtnPlus = lv_label_create(btnPlus);
  lv_label_set_text(txtBtnPlus, "+");
  lv_obj_center(txtBtnPlus);
  lv_obj_add_event_cb(btnPlus, event_handler, LV_EVENT_ALL, btnPlus->user_data);

  btnMinus = lv_btn_create(lv_scr_act());
  btnMinus->user_data = this;
  lv_obj_set_size(btnMinus, 80, 50);
  lv_obj_align(btnMinus, LV_ALIGN_CENTER, -55, 80);
  txtBtnMinus = lv_label_create(btnMinus);
  lv_label_set_text(txtBtnMinus, "-");
  lv_obj_center(txtBtnMinus);
  lv_obj_add_event_cb(btnMinus, event_handler, LV_EVENT_ALL, btnMinus->user_data);

}

SettingSteps::~SettingSteps() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingSteps::UpdateSelected(lv_obj_t *object, lv_event_t* event) {
  uint32_t value = settingsController.GetStepsGoal();
  if(object == btnPlus && (lv_event_get_code(event) == LV_EVENT_PRESSED)) {
    value += 1000;
    if ( value <= 500000 ) {
      settingsController.SetStepsGoal(value);
      lv_label_set_text_fmt(stepValue, "%lu", settingsController.GetStepsGoal());
      lv_obj_align(stepValue, LV_ALIGN_CENTER, 0, -10);
    }
  }

  if(object == btnMinus && (lv_event_get_code(event) == LV_EVENT_PRESSED)) {
    value -= 1000;
    if ( value >= 1000 ) {
      settingsController.SetStepsGoal(value);
      lv_label_set_text_fmt(stepValue, "%lu", settingsController.GetStepsGoal());
      lv_obj_align(stepValue, LV_ALIGN_CENTER, 0, -10);
    }
  }

}
