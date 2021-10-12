#include "Timer.h"

#include "Screen.h"
#include "Symbols.h"
#include "lvgl/lvgl.h"

using namespace Pinetime::Applications::Screens;

static void btnEventHandler(lv_event_t* event) {
  Timer* screen = static_cast<Timer*>(lv_event_get_user_data(event));
  screen->OnButtonEvent(lv_event_get_target(event), event);
}

void Timer::createButtons() {
  btnMinutesUp = lv_btn_create(lv_scr_act());
  btnMinutesUp->user_data = this;
  lv_obj_add_event_cb(btnMinutesUp, btnEventHandler, LV_EVENT_ALL, btnMinutesUp->user_data);
  lv_obj_set_size(btnMinutesUp, 60, 40);
  lv_obj_align(btnMinutesUp, LV_ALIGN_LEFT_MID, 20, -85);
  txtMUp = lv_label_create(btnMinutesUp);
  lv_label_set_text(txtMUp, "+");
  lv_obj_center(txtMUp);

  btnMinutesDown = lv_btn_create(lv_scr_act());
  btnMinutesDown->user_data = this;
  lv_obj_add_event_cb(btnMinutesDown, btnEventHandler, LV_EVENT_ALL, btnMinutesDown->user_data);
  lv_obj_set_size(btnMinutesDown, 60, 40);
  lv_obj_align(btnMinutesDown, LV_ALIGN_LEFT_MID, 20, 35);
  txtMDown = lv_label_create(btnMinutesDown);
  lv_label_set_text(txtMDown, "-");
  lv_obj_center(txtMDown);

  btnSecondsUp = lv_btn_create(lv_scr_act());
  btnSecondsUp->user_data = this;
  lv_obj_add_event_cb(btnSecondsUp, btnEventHandler, LV_EVENT_ALL, btnSecondsUp->user_data);
  lv_obj_set_size(btnSecondsUp, 60, 40);
  lv_obj_align(btnSecondsUp, LV_ALIGN_RIGHT_MID, -20, -85);
  txtSUp = lv_label_create(btnSecondsUp);
  lv_label_set_text(txtSUp, "+");
  lv_obj_center(txtSUp);

  btnSecondsDown = lv_btn_create(lv_scr_act());
  btnSecondsDown->user_data = this;
  lv_obj_add_event_cb(btnSecondsDown, btnEventHandler, LV_EVENT_ALL, btnSecondsDown->user_data);
  lv_obj_set_size(btnSecondsDown, 60, 40);
  lv_obj_align(btnSecondsDown, LV_ALIGN_RIGHT_MID, -20, 35);
  txtSDown = lv_label_create(btnSecondsDown);
  lv_label_set_text(txtSDown, "-");
  lv_obj_center(txtSDown);
}

Timer::Timer(DisplayApp* app, Controllers::TimerController& timerController)
  : Screen(app), running {true}, timerController {timerController} {

  time = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(time, &jetbrains_mono_76, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(time, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT);

  uint32_t seconds = timerController.GetTimeRemaining() / 1000;
  lv_label_set_text_fmt(time, "%02lu:%02lu", seconds / 60, seconds % 60);

  lv_obj_align(time, LV_ALIGN_CENTER, 0, -25);

  btnPlayPause = lv_btn_create(lv_scr_act());
  btnPlayPause->user_data = this;
  lv_obj_add_event_cb(btnPlayPause, btnEventHandler, LV_EVENT_ALL, btnPlayPause->user_data);
  lv_obj_align(btnPlayPause, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_set_height(btnPlayPause, 40);
  txtPlayPause = lv_label_create(btnPlayPause);
  if (timerController.IsRunning()) {
    lv_label_set_text(txtPlayPause, Symbols::pause);
  } else {
    lv_label_set_text(txtPlayPause, Symbols::play);
    createButtons();
  }

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, this);
}

Timer::~Timer() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Timer::Refresh() {
  if (timerController.IsRunning()) {
    uint32_t seconds = timerController.GetTimeRemaining() / 1000;
    uint8_t curMinutes = seconds / 60;
    uint8_t curSeconds = seconds % 60;
    // Avoids constant redrawing
    if (curMinutes != pMinutes || curSeconds != pSeconds) {
      pMinutes = curMinutes;
      pSeconds = curSeconds;
      lv_label_set_text_fmt(time, "%02u:%02u", curMinutes, curSeconds);
    }
  }
}

void Timer::OnButtonEvent(lv_obj_t* obj, lv_event_t* event) {
  if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
    if (obj == btnPlayPause) {
      if (timerController.IsRunning()) {
        lv_label_set_text(txtPlayPause, Symbols::play);
        uint32_t seconds = timerController.GetTimeRemaining() / 1000;
        minutesToSet = seconds / 60;
        secondsToSet = seconds % 60;
        timerController.StopTimer();
        createButtons();

      } else if (secondsToSet + minutesToSet > 0) {
        lv_label_set_text(txtPlayPause, Symbols::pause);
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

void Timer::setDone() {
  lv_label_set_text(time, "00:00");
  lv_label_set_text(txtPlayPause, Symbols::play);
  secondsToSet = 0;
  minutesToSet = 0;
  createButtons();
}
