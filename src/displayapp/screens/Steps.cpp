#include "displayapp/screens/Steps.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

using Days = Pinetime::Controllers::MotionController::Days;

static void lap_event_handler(lv_obj_t* obj, lv_event_t event) {
  auto* steps = static_cast<Steps*>(obj->user_data);
  steps->lapBtnEventHandler(event);
}

Steps::Steps(Controllers::MotionController& motionController, Controllers::Settings& settingsController)
  : motionController {motionController}, settingsController {settingsController} {

  stepsArc = lv_arc_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(stepsArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_style_local_line_color(stepsArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_style_local_border_width(stepsArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_radius(stepsArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_line_color(stepsArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, Colors::blue);
  lv_arc_set_end_angle(stepsArc, 200);
  lv_obj_set_size(stepsArc, 240, 240);
  lv_arc_set_range(stepsArc, 0, 500);
  lv_obj_align(stepsArc, nullptr, LV_ALIGN_CENTER, 0, 0);

  lSteps = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lSteps, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_obj_set_style_local_text_font(lSteps, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_align(lSteps, nullptr, LV_ALIGN_CENTER, 0, -40);
  lv_obj_set_auto_realign(lSteps, true);

  lv_obj_t* lstepsL = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lstepsL, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(lstepsL, "Steps");
  lv_obj_align(lstepsL, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  lStepsYesterday = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lStepsYesterday, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_align(lStepsYesterday, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lStepsYesterday, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
  lv_obj_set_auto_realign(lStepsYesterday, true);

  lStepsGoal = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lStepsGoal, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
  lv_label_set_align(lStepsGoal, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lStepsGoal, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);
  lv_obj_set_auto_realign(lStepsGoal, true);

  resetBtn = lv_btn_create(lv_scr_act(), nullptr);
  resetBtn->user_data = this;
  lv_obj_set_event_cb(resetBtn, lap_event_handler);
  lv_obj_set_size(resetBtn, 120, 50);
  lv_obj_set_style_local_radius(resetBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(resetBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_align(resetBtn, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  resetButtonLabel = lv_label_create(resetBtn, nullptr);
  lv_label_set_text_static(resetButtonLabel, "Reset");

  tripLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(tripLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_align(tripLabel, lStepsGoal, LV_ALIGN_IN_LEFT_MID, 0, 20);
  lv_obj_set_auto_realign(tripLabel, true);

  Refresh();
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Steps::~Steps() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Steps::Refresh() {
  stepsCount = motionController.NbSteps();
  currentTripSteps = motionController.GetTripSteps();
  stepsGoal = settingsController.GetStepsGoal();

  if (stepsCount.IsUpdated() || stepsGoal.IsUpdated()) {
    lv_label_set_text_fmt(lSteps, "%lu", stepsCount.Get());
    lv_label_set_text_fmt(lStepsGoal, "Goal: %5lu", stepsGoal.Get());
    lv_label_set_text_fmt(lStepsYesterday, "Yest: %5lu", motionController.NbSteps(Days::Yesterday));
    lv_arc_set_value(stepsArc, int16_t(500 * stepsCount.Get() / stepsGoal.Get()));
  }

  if (currentTripSteps.IsUpdated()) {
    if (currentTripSteps.Get() < 100000) {
      lv_label_set_text_fmt(tripLabel, "Trip: %5li", currentTripSteps.Get());
    } else {
      lv_label_set_text_fmt(tripLabel, "Trip: 99999+");
    }
  }
}

void Steps::lapBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  motionController.ResetTrip();
  Refresh();
}
