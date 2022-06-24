#include "random"
#include "functional"
#include "displayapp/screens/Dice.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Dice*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

Dice::Dice(DisplayApp* app, Controllers::Settings& settingsController, Controllers::MotionController& motionController)
  : Screen(app), settingsController {settingsController}, motionController {motionController} {

  count = 1;
  grade = 20;
  substractor = 0;

  txtCount = lv_label_create(lv_scr_act(), nullptr);
  txtGradeSeparator = lv_label_create(lv_scr_act(), nullptr);
  txtGrade = lv_label_create(lv_scr_act(), nullptr);
  txtSubstractorSeparator = lv_label_create(lv_scr_act(), nullptr);
  txtSubstractor = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(txtCount, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_style_local_text_font(txtGrade, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_style_local_text_font(txtSubstractor, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_style_local_text_color(txtCount, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb0, 0xb0, 0xb0));
  lv_obj_set_style_local_text_color(txtGrade, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb0, 0xb0, 0xb0));
  lv_obj_set_style_local_text_color(txtSubstractor, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb0, 0xb0, 0xb0));
  lv_label_set_text_static(txtGradeSeparator, "d");
  lv_label_set_text_static(txtSubstractorSeparator, "-");

  UpdateRollConfig();

  lv_obj_align(txtCount, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 22, -25);
  lv_obj_align(txtGradeSeparator, lv_scr_act(), LV_ALIGN_CENTER, -36, -25);
  lv_obj_align(txtGrade, lv_scr_act(), LV_ALIGN_CENTER, 0, -25);
  lv_obj_align(txtSubstractorSeparator, lv_scr_act(), LV_ALIGN_CENTER, 36, -25);
  lv_obj_align(txtSubstractor, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -22, -25);

  lblampm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblampm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(lblampm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb0, 0xb0, 0xb0));
  lv_label_set_text_static(lblampm, "  ");
  lv_label_set_align(lblampm, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblampm, lv_scr_act(), LV_ALIGN_CENTER, 0, 30);

  btnCountUp = lv_btn_create(lv_scr_act(), nullptr);
  btnCountUp->user_data = this;
  lv_obj_set_event_cb(btnCountUp, btnEventHandler);
  lv_obj_set_size(btnCountUp, 60, 40);
  lv_obj_align(btnCountUp, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 20, -85);
  txtCountUp = lv_label_create(btnCountUp, nullptr);
  lv_label_set_text_static(txtCountUp, "+");

  btnCountDown = lv_btn_create(lv_scr_act(), nullptr);
  btnCountDown->user_data = this;
  lv_obj_set_event_cb(btnCountDown, btnEventHandler);
  lv_obj_set_size(btnCountDown, 60, 40);
  lv_obj_align(btnCountDown, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 20, 35);
  txtCountDown = lv_label_create(btnCountDown, nullptr);
  lv_label_set_text_static(txtCountDown, "-");

  btnGradeUp = lv_btn_create(lv_scr_act(), nullptr);
  btnGradeUp->user_data = this;
  lv_obj_set_event_cb(btnGradeUp, btnEventHandler);
  lv_obj_set_size(btnGradeUp, 60, 40);
  lv_obj_align(btnGradeUp, lv_scr_act(), LV_ALIGN_CENTER, 0, -85);
  txtGradeUp = lv_label_create(btnGradeUp, nullptr);
  lv_label_set_text_static(txtGradeUp, "+");

  btnGradeDown = lv_btn_create(lv_scr_act(), nullptr);
  btnGradeDown->user_data = this;
  lv_obj_set_event_cb(btnGradeDown, btnEventHandler);
  lv_obj_set_size(btnGradeDown, 60, 40);
  lv_obj_align(btnGradeDown, lv_scr_act(), LV_ALIGN_CENTER, 0, 35);
  txtGradeDown = lv_label_create(btnGradeDown, nullptr);
  lv_label_set_text_static(txtGradeDown, "-");

  btnSubstractorUp = lv_btn_create(lv_scr_act(), nullptr);
  btnSubstractorUp->user_data = this;
  lv_obj_set_event_cb(btnSubstractorUp, btnEventHandler);
  lv_obj_set_size(btnSubstractorUp, 60, 40);
  lv_obj_align(btnSubstractorUp, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -20, -85);
  txtSubstractorUp = lv_label_create(btnSubstractorUp, nullptr);
  lv_label_set_text_static(txtSubstractorUp, "+");

  btnSubstractorDown = lv_btn_create(lv_scr_act(), nullptr);
  btnSubstractorDown->user_data = this;
  lv_obj_set_event_cb(btnSubstractorDown, btnEventHandler);
  lv_obj_set_size(btnSubstractorDown, 60, 40);
  lv_obj_align(btnSubstractorDown, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -20, 35);
  txtSubstractorDown = lv_label_create(btnSubstractorDown, nullptr);
  lv_label_set_text_static(txtSubstractorDown, "-");

  btnRoll = lv_btn_create(lv_scr_act(), nullptr);
  btnRoll->user_data = this;
  lv_obj_set_event_cb(btnRoll, btnEventHandler);
  lv_obj_set_size(btnRoll, 115, 50);
  lv_obj_align(btnRoll, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  txtRoll = lv_label_create(btnRoll, nullptr);
  lv_label_set_text_static(txtRoll, "Roll");

  refreshTask = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Dice::~Dice() {
  lv_task_del(refreshTask);
  lv_obj_clean(lv_scr_act());
}

void Dice::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnRoll) {
      ShowRoll();
      return;
    }
    if (obj == btnMessage) {
      HideRoll();
      return;
    }
    if (obj == btnCountUp) {
      if (count >= 99) {
        count = 1;
      } else {
        count++;
      }
      UpdateRollConfig();
      return;
    }
    if (obj == btnCountDown) {
      if (count == 1) {
        count = 99;
      } else {
        count--;
      }
      UpdateRollConfig();
      return;
    }
    if (obj == btnGradeUp) {
      if (grade >= 20) {
        grade = 2;
      } else {
        grade++;
      }
      UpdateRollConfig();
      return;
    }
    if (obj == btnGradeDown) {
      if (grade == 2) {
        grade = 20;
      } else {
        grade--;
      }
      UpdateRollConfig();
      return;
    }
    if (obj == btnSubstractorUp) {
      if (substractor >= 99) {
        substractor = 0;
      } else {
        substractor++;
      }
      UpdateRollConfig();
      return;
    }
    if (obj == btnSubstractorDown) {
      if (substractor == 0) {
        substractor = 99;
      } else {
        substractor--;
      }
      UpdateRollConfig();
      return;
    }
  }
}

bool Dice::OnButtonPushed() {
  if (txtMessage != nullptr && btnMessage != nullptr) {
    HideRoll();
    return true;
  }
  return false;
}

void Dice::UpdateRollConfig() {
  lv_label_set_text_fmt(txtCount, "%02u", count);
  lv_label_set_text_fmt(txtGrade, "%02u", grade);
  lv_label_set_text_fmt(txtSubstractor, "%02u", substractor);
}

void Dice::ShowRoll() {
  if (txtMessage != nullptr && btnMessage != nullptr) {
    HideRoll();
  }
  btnMessage = lv_btn_create(lv_scr_act(), nullptr);
  btnMessage->user_data = this;
  lv_obj_set_event_cb(btnMessage, btnEventHandler);
  lv_obj_set_height(btnMessage, 200);
  lv_obj_set_width(btnMessage, 150);
  lv_obj_align(btnMessage, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  txtMessage = lv_label_create(btnMessage, nullptr);
  lv_obj_set_style_local_bg_color(btnMessage, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_NAVY);
  lv_label_set_text_fmt(txtMessage, "Rolled:\n%2u", Roll());
}

void Dice::HideRoll() {
  lv_obj_del(btnMessage);
  txtMessage = nullptr;
  btnMessage = nullptr;
}

uint8_t Dice::Roll() {
  std::seed_seq seed {static_cast<uint16_t>(std::random_device()() % 65535),
                      static_cast<uint16_t>((motionController.X() + motionController.Y() + motionController.Z()) % 32767)};
  std::default_random_engine generator {seed};
  std::uniform_int_distribution<uint8_t> distribution(1, grade);
  auto dice = std::bind(distribution, generator);
  uint8_t sum = 0;
  for (uint8_t i = 1; i <= count; i++) {
    sum += dice();
  }
  if (substractor > sum) {
    return 0;
  }
  return sum - substractor;
}

void Dice::Refresh() {
  if (motionController.Should_ShakeWake(std::min(settingsController.GetShakeThreshold() * 4, 65535))) {
    ShowRoll();
  }
}
