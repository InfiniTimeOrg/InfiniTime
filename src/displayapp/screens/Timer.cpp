#include "displayapp/screens/Timer.h"

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  Timer* screen = static_cast<Timer*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

void Timer::createButtons() {
  btnMinutesUp = lv_btn_create(lv_scr_act(), nullptr);
  btnMinutesUp->user_data = this;
  lv_obj_set_event_cb(btnMinutesUp, btnEventHandler);
  lv_obj_set_size(btnMinutesUp, 60, 40);
  lv_obj_align(btnMinutesUp, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 20, -85);
  txtMUp = lv_label_create(btnMinutesUp, nullptr);
  lv_label_set_text(txtMUp, "+");

  btnMinutesDown = lv_btn_create(lv_scr_act(), nullptr);
  btnMinutesDown->user_data = this;
  lv_obj_set_event_cb(btnMinutesDown, btnEventHandler);
  lv_obj_set_size(btnMinutesDown, 60, 40);
  lv_obj_align(btnMinutesDown, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 20, +35);
  txtMDown = lv_label_create(btnMinutesDown, nullptr);
  lv_label_set_text(txtMDown, "-");

  btnSecondsUp = lv_btn_create(lv_scr_act(), nullptr);
  btnSecondsUp->user_data = this;
  lv_obj_set_event_cb(btnSecondsUp, btnEventHandler);
  lv_obj_set_size(btnSecondsUp, 60, 40);
  lv_obj_align(btnSecondsUp, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -20, -85);
  txtSUp = lv_label_create(btnSecondsUp, nullptr);
  lv_label_set_text(txtSUp, "+");

  btnSecondsDown = lv_btn_create(lv_scr_act(), nullptr);
  btnSecondsDown->user_data = this;
  lv_obj_set_event_cb(btnSecondsDown, btnEventHandler);
  lv_obj_set_size(btnSecondsDown, 60, 40);
  lv_obj_align(btnSecondsDown, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -20, +35);
  txtSDown = lv_label_create(btnSecondsDown, nullptr);
  lv_label_set_text(txtSDown, "-");
}

void Timer::stop() {
  int32_t secondsRemaining = timerController.GetSecondsRemaining();
  if (timerController.IsOvertime()) {
    minutesToSet = 0;
    secondsToSet = 0;
    secondsRemaining = 0;
  } else {
    minutesToSet = secondsRemaining / 60;
    secondsToSet = secondsRemaining % 60;
  }
  timerController.StopTimer();
  timerController.StopAlerting();
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text_fmt(time, "%02lu:%02lu", secondsRemaining / 60, secondsRemaining % 60);
  lv_label_set_text(txtPlayPause, Symbols::play);
  createButtons();
}

Timer::Timer(DisplayApp* app, Controllers::TimerController& timerController)
  : Screen(app), running {true}, timerController {timerController} {

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);

  int32_t seconds = timerController.GetSecondsRemaining();
  bool overtime = timerController.IsOvertime();

  if (overtime) {
    seconds = -seconds + 1; // "+ 1" is to not show -00:00 again after +00:00
    lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  } else {
    lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  }

  lv_label_set_text_fmt(time, "%02lu:%02lu", seconds / 60, seconds % 60);

  lv_obj_align(time, lv_scr_act(), LV_ALIGN_CENTER, 0, -25);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, btnEventHandler);
  lv_obj_set_size(btnPlayPause, 115, 50);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  if (timerController.IsRunning()) {
    lv_label_set_text(txtPlayPause, overtime ? Symbols::stop : Symbols::pause);
  } else {
    lv_label_set_text(txtPlayPause, Symbols::play);
    createButtons();
  }
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Timer::~Timer() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
  if (timerController.IsRunning() && timerController.IsOvertime()) {
    timerController.StopTimer();
    timerController.StopAlerting();
  }
}

void Timer::Refresh() {
  if (timerController.IsRunning()) {
    int32_t seconds = timerController.GetSecondsRemaining();
    if (timerController.IsOvertime()) {
      seconds = -seconds + 1; // "+ 1" is to not show -00:00 again after +00:00

      // safety measures, lets not overflow counter as it will display badly
      if (seconds >= 100 * 60) {
        stop();
        return;
      }
    }
    lv_label_set_text_fmt(time, "%02lu:%02lu", seconds / 60, seconds % 60);
  }
}

void Timer::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnPlayPause) {
      if (timerController.IsRunning()) {
        stop();
      } else if (secondsToSet + minutesToSet > 0) {
        lv_label_set_text(txtPlayPause, Symbols::pause);
        timerController.StartTimer((secondsToSet + minutesToSet * 60) * 1000);

        // inlined destroyButtons()
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

void Timer::setDone() {
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_label_set_text(txtPlayPause, Symbols::stop);
}
