#include "Steps.h"
#include <lvgl/lvgl.h>
#include "../DisplayApp.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

Steps::Steps(Pinetime::Applications::DisplayApp* app,
             Controllers::MotionController& motionController,
             Controllers::Settings& settingsController)
  : Screen(app), motionController {motionController}, settingsController {settingsController} {

  stepsArc = lv_arc_create(lv_scr_act());

  lv_obj_set_style_bg_opa(stepsArc, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(stepsArc, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(stepsArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_line_color(stepsArc, lv_color_hex(0x0000FF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_arc_set_end_angle(stepsArc, 200);
  lv_obj_set_size(stepsArc, 220, 220);
  lv_arc_set_range(stepsArc, 0, 500);
  lv_obj_align(stepsArc,  LV_ALIGN_CENTER, 0, 0);
  lv_obj_clear_flag(stepsArc, LV_OBJ_FLAG_CLICKABLE);

  stepsCount = motionController.NbSteps();

  lv_arc_set_value(stepsArc, int16_t(500 * stepsCount / settingsController.GetStepsGoal()));

  lSteps = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(lSteps, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(lSteps, &jetbrains_mono_42, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_fmt(lSteps, "%li", stepsCount);
  lv_obj_align(lSteps, LV_ALIGN_CENTER, 0, -20);

  lv_obj_t* lstepsL = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(lstepsL, lv_color_hex(0x111111), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(lstepsL, "Steps");
  lv_obj_align_to(lstepsL, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  lv_obj_t* lstepsGoal = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(lstepsGoal, lv_color_hex(0x00FFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_fmt(lstepsGoal, "Goal\n%lu", settingsController.GetStepsGoal());
  lv_obj_set_style_text_align(lstepsGoal, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align_to(lstepsGoal, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 60);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  taskRefresh = lv_timer_create(RefreshTaskCallback, 100, this);
}

Steps::~Steps() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Steps::Refresh() {
  if (motionController.NbSteps() != stepsCount) {
    stepsCount = motionController.NbSteps();

    lv_label_set_text_fmt(lSteps, "%li", stepsCount);
    lv_obj_align(lSteps, LV_ALIGN_CENTER, 0, -20);

    lv_arc_set_value(stepsArc, int16_t(500 * stepsCount / settingsController.GetStepsGoal()));
  }
}
