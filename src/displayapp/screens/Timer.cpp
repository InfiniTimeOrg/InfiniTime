#include "displayapp/screens/Timer.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Timer*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

void Timer::CreateButtons() {
  btnMinutesUp = lv_btn_create(lv_scr_act(), nullptr);
  btnMinutesUp->user_data = this;
  lv_obj_set_event_cb(btnMinutesUp, btnEventHandler);
  lv_obj_align(btnMinutesUp, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 20, -80);
  lv_obj_set_height(btnMinutesUp, 40);
  lv_obj_set_width(btnMinutesUp, 60);
  txtMUp = lv_label_create(btnMinutesUp, nullptr);
  lv_label_set_text_static(txtMUp, "+");

  btnMinutesDown = lv_btn_create(lv_scr_act(), nullptr);
  btnMinutesDown->user_data = this;
  lv_obj_set_event_cb(btnMinutesDown, btnEventHandler);
  lv_obj_align(btnMinutesDown, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 20, +40);
  lv_obj_set_height(btnMinutesDown, 40);
  lv_obj_set_width(btnMinutesDown, 60);
  txtMDown = lv_label_create(btnMinutesDown, nullptr);
  lv_label_set_text_static(txtMDown, "-");

  btnSecondsUp = lv_btn_create(lv_scr_act(), nullptr);
  btnSecondsUp->user_data = this;
  lv_obj_set_event_cb(btnSecondsUp, btnEventHandler);
  lv_obj_align(btnSecondsUp, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 10, -80);
  lv_obj_set_height(btnSecondsUp, 40);
  lv_obj_set_width(btnSecondsUp, 60);
  txtSUp = lv_label_create(btnSecondsUp, nullptr);
  lv_label_set_text_static(txtSUp, "+");

  btnSecondsDown = lv_btn_create(lv_scr_act(), nullptr);
  btnSecondsDown->user_data = this;
  lv_obj_set_event_cb(btnSecondsDown, btnEventHandler);
  lv_obj_align(btnSecondsDown, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 10, +40);
  lv_obj_set_height(btnSecondsDown, 40);
  lv_obj_set_width(btnSecondsDown, 60);
  txtSDown = lv_label_create(btnSecondsDown, nullptr);
  lv_label_set_text_static(txtSDown, "-");
}

Timer::Timer(DisplayApp* app, Controllers::TimerController& timerController)
  : Screen(app), running {true}, timerController {timerController} {
  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  uint32_t seconds = timerController.GetTimeRemaining() / 1000;
  lv_label_set_text_fmt(time, "%02lu:%02lu", seconds / 60, seconds % 60);

  lv_obj_align(time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -20);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, btnEventHandler);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_height(btnPlayPause, 40);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  if (timerController.IsRunning()) {
    lv_label_set_text_static(txtPlayPause, Symbols::pause);
  } else {
    lv_label_set_text_static(txtPlayPause, Symbols::play);
    CreateButtons();
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Timer::~Timer() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Timer::Refresh() {
  if (timerController.IsRunning()) {
    uint32_t seconds = timerController.GetTimeRemaining() / 1000;
    lv_label_set_text_fmt(time, "%02lu:%02lu", seconds / 60, seconds % 60);
  }
}

void Timer::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnPlayPause) {
      if (timerController.IsRunning()) {
        lv_label_set_text_static(txtPlayPause, Symbols::play);
        uint32_t seconds = timerController.GetTimeRemaining() / 1000;
        minutesToSet = seconds / 60;
        secondsToSet = seconds % 60;
        timerController.StopTimer();
        CreateButtons();

      } else if (secondsToSet + minutesToSet > 0) {
        lv_label_set_text_static(txtPlayPause, Symbols::pause);
        timerController.StartTimer((secondsToSet + minutesToSet * 60) * 1000);

        lv_obj_del(btnSecondsDown);
        btnSecondsDown = nullptr;
        lv_obj_del(btnSecondsUp);
        btnSecondsUp = nullptr;
        lv_obj_del(btnMinutesDown);
        btnMinutesDown = nullptr;
        lv_obj_del(btnMinutesUp);
        btnMinutesUp = nullptr;
      }
    } else {
      if (!timerController.IsRunning()) {
        if (obj == btnMinutesUp) {
          if (minutesToSet >= 59) {
            minutesToSet = 0;
          } else {
            minutesToSet++;
          }
          lv_label_set_text_fmt(time, "%02d:%02d", minutesToSet, secondsToSet);

        } else if (obj == btnMinutesDown) {
          if (minutesToSet == 0) {
            minutesToSet = 59;
          } else {
            minutesToSet--;
          }
          lv_label_set_text_fmt(time, "%02d:%02d", minutesToSet, secondsToSet);

        } else if (obj == btnSecondsUp) {
          if (secondsToSet >= 59) {
            secondsToSet = 0;
          } else {
            secondsToSet++;
          }
          lv_label_set_text_fmt(time, "%02d:%02d", minutesToSet, secondsToSet);

        } else if (obj == btnSecondsDown) {
          if (secondsToSet == 0) {
            secondsToSet = 59;
          } else {
            secondsToSet--;
          }
          lv_label_set_text_fmt(time, "%02d:%02d", minutesToSet, secondsToSet);
        }
      }
    }
  }
}

void Timer::SetDone() {
  lv_label_set_text_static(time, "00:00");
  lv_label_set_text_static(txtPlayPause, Symbols::play);
  secondsToSet = 0;
  minutesToSet = 0;
  CreateButtons();
}
