#include "displayapp/screens/Steps.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Steps*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
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
  lv_arc_set_bg_angles(stepsArc, 160, 20);
  lv_obj_set_size(stepsArc, 240, 240);
  lv_arc_set_range(stepsArc, 0, 500);
  lv_obj_align(stepsArc, nullptr, LV_ALIGN_CENTER, 0, 0);

  stepsCount = motionController.NbSteps();
  currentTripSteps = stepsCount - motionController.GetTripSteps();

  lv_arc_set_value(stepsArc, int16_t(500 * stepsCount / settingsController.GetStepsGoal()));

  lSteps = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lSteps, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FF00));
  lv_obj_set_style_local_text_font(lSteps, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_fmt(lSteps, "%li", stepsCount);
  lv_obj_align(lSteps, nullptr, LV_ALIGN_CENTER, 0, -40);

  lstepsL = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lstepsL, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
  lv_label_set_text_static(lstepsL, "Steps");
  lv_obj_align(lstepsL, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  lstepsGoal = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lstepsGoal, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
  lv_label_set_text_fmt(lstepsGoal, "Goal: %5lu", settingsController.GetStepsGoal());
  lv_label_set_align(lstepsGoal, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lstepsGoal, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);

  tripLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(tripLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_label_set_text_fmt(tripLabel, "Trip: %5li", currentTripSteps);
  lv_obj_align(tripLabel, lstepsGoal, LV_ALIGN_IN_LEFT_MID, 0, 20);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  btnReset = lv_btn_create(lv_scr_act(), nullptr);
  btnReset->user_data = this;
  lv_obj_set_event_cb(btnReset, event_handler);
  lv_obj_set_height(btnReset, 50);
  lv_obj_set_width(btnReset, 115);
  lv_obj_align(btnReset, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, -60, -5);
  btnResetLabel = lv_label_create(btnReset, nullptr);
  lv_label_set_text(btnResetLabel, "Reset");

  btnSet = lv_btn_create(lv_scr_act(), nullptr);
  btnSet->user_data = this;
  lv_obj_set_event_cb(btnSet, event_handler);
  lv_obj_set_height(btnSet, 50);
  lv_obj_set_width(btnSet, 115);
  lv_obj_align(btnSet, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 60, -5);
  btnSetLabel = lv_label_create(btnSet, nullptr);
  lv_label_set_text(btnSetLabel, "Set goal");

  btnPlus = lv_btn_create(lv_scr_act(), nullptr);
  btnPlus->user_data = this;
  lv_obj_set_size(btnPlus, 60, 60);
  lv_obj_align(btnPlus, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 0);
  lv_obj_set_style_local_bg_opa(btnPlus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnPlus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");
  lv_obj_set_event_cb(btnPlus, event_handler);
  lv_obj_set_hidden(btnPlus, true);

  btnMinus = lv_btn_create(lv_scr_act(), nullptr);
  btnMinus->user_data = this;
  lv_obj_set_size(btnMinus, 60, 60);
  lv_obj_align(btnMinus, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 0);
  lv_obj_set_style_local_bg_opa(btnMinus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnMinus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");
  lv_obj_set_event_cb(btnMinus, event_handler);
  lv_obj_set_hidden(btnMinus, true);

  btnClose = lv_btn_create(lv_scr_act(), nullptr);
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "X");
  lv_obj_set_event_cb(btnClose, event_handler);
  lv_obj_set_hidden(btnClose, true);

  currentTripSteps = motionController.GetTripSteps();

  taskRefresh = lv_task_create(RefreshTaskCallback, 100, LV_TASK_PRIO_MID, this);
}

Steps::~Steps() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Steps::Refresh() {
  stepsCount = motionController.NbSteps();
  currentTripSteps = motionController.GetTripSteps();

  lv_label_set_text_fmt(lSteps, "%li", stepsCount);
  lv_obj_align(lSteps, nullptr, LV_ALIGN_CENTER, 0, -40);

  if (currentTripSteps < 100000) {
    lv_label_set_text_fmt(tripLabel, "Trip: %5li", currentTripSteps);
  } else {
    lv_label_set_text_fmt(tripLabel, "Trip: 99999+");
  }
  lv_arc_set_value(stepsArc, int16_t(500 * stepsCount / settingsController.GetStepsGoal()));
}

void Steps::UpdateSelected(lv_obj_t *object, lv_event_t event) {
  uint32_t value = settingsController.GetStepsGoal();
  if (event == LV_EVENT_CLICKED) {
    if(object == btnPlus) {
    value += 1000;
      if ( value <= 500000 ) {
        settingsController.SetStepsGoal(value);
        lv_label_set_text_fmt(lstepsGoal, "Goal: %5lu", settingsController.GetStepsGoal());
        lv_obj_realign(lstepsGoal);
      }
    }
    if(object == btnMinus) {
      value -= 1000;
      if ( value >= 1000 ) {
        settingsController.SetStepsGoal(value);
        lv_label_set_text_fmt(lstepsGoal, "Goal: %5lu", settingsController.GetStepsGoal());
        lv_obj_realign(lstepsGoal);
      }
    }
    if (object == btnClose) {
      lv_obj_set_hidden(btnPlus, true);
      lv_obj_set_hidden(btnMinus, true);
      lv_obj_set_hidden(btnClose, true);
    }
    if (object == btnSet) {
      lv_obj_set_hidden(btnPlus, false);
      lv_obj_set_hidden(btnMinus, false);
      lv_obj_set_hidden(btnClose, false);
    }
    if (object == btnReset) {
      stepsCount = motionController.NbSteps();
      motionController.ResetTrip();
      Refresh();
    }
  }
}