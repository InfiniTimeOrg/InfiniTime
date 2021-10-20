#include "displayapp/screens/Steps.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

static void lap_event_handler(lv_obj_t* obj, lv_event_t event) {
  auto* steps = static_cast<Steps*>(obj->user_data);
  steps->lapBtnEventHandler(event);
}

Steps::Steps(Pinetime::Applications::DisplayApp* app,
             Controllers::MotionController& motionController,
             Controllers::Settings& settingsController)
  : Screen(app), motionController {motionController}, settingsController {settingsController} {

  stepsArc = lv_arc_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(stepsArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_style_local_border_width(stepsArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_radius(stepsArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_line_color(stepsArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0x0000FF));
  lv_arc_set_end_angle(stepsArc, 200);
  lv_obj_set_size(stepsArc, 200, 200);
  lv_arc_set_range(stepsArc, 0, 500);
  lv_obj_align(stepsArc, nullptr, LV_ALIGN_CENTER, 0, -20);

  stepsCount = motionController.NbSteps();
  currentLapSteps = stepsCount - motionController.GetPrevTotalSteps();

  lv_arc_set_value(stepsArc, int16_t(500 * stepsCount / settingsController.GetStepsGoal()));

  lSteps = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lSteps, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FF00));
  lv_obj_set_style_local_text_font(lSteps, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_fmt(lSteps, "%li", currentLapSteps);
  lv_obj_align(lSteps, nullptr, LV_ALIGN_CENTER, 0, -40);

  lv_obj_t* lstepsL = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lstepsL, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
  lv_label_set_text_static(lstepsL, "Steps");
  lv_obj_align(lstepsL, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  lv_obj_t* lstepsGoal = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lstepsGoal, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
  lv_label_set_text_fmt(lstepsGoal, "Goal\n%lu", settingsController.GetStepsGoal());
  lv_label_set_align(lstepsGoal, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lstepsGoal, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  btnLap = lv_btn_create(lv_scr_act(), nullptr);
  btnLap->user_data = this;
  lv_obj_set_event_cb(btnLap, lap_event_handler);
  lv_obj_set_height(btnLap, 50);
  lv_obj_set_width(btnLap, 115);
  lv_obj_align(btnLap, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(btnLap, LV_BTN_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x080808));
  txtLap = lv_label_create(btnLap, nullptr);
  lv_obj_set_style_local_text_color(btnLap, LV_BTN_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x888888));
  lv_label_set_text(txtLap, Symbols::lapsFlag);

  totalStepsText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(totalStepsText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_label_set_text_fmt(totalStepsText, "Total\n%li", motionController.GetPrevTotalSteps());
  lv_obj_align(totalStepsText, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  taskRefresh = lv_task_create(RefreshTaskCallback, 100, LV_TASK_PRIO_MID, this);
}

Steps::~Steps() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Steps::Refresh() {
  stepsCount = motionController.NbSteps();
  currentLapSteps = stepsCount - motionController.GetPrevTotalSteps();

  lv_label_set_text_fmt(lSteps, "%li", currentLapSteps);
  lv_obj_align(lSteps, nullptr, LV_ALIGN_CENTER, 0, -40);

  lv_label_set_text_fmt(totalStepsText, "Total\n%li", stepsCount);
  lv_obj_align(totalStepsText, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  lv_arc_set_value(stepsArc, int16_t(500 * stepsCount / settingsController.GetStepsGoal()));
}

void Steps::lapBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  stepsCount = motionController.NbSteps();
  motionController.SetPrevTotalSteps(stepsCount);
  Refresh();
}

