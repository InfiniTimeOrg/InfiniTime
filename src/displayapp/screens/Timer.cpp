#include "displayapp/screens/Timer.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Timer*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

Timer::Timer(DisplayApp* app, Controllers::TimerController& timerController) : Screen(app), timerController {timerController} {

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(time, "00:00");
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_CENTER, 0, -29);

  minuteCounter.Create();
  secondCounter.Create();
  lv_obj_align(minuteCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_align(secondCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_style_local_radius(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x38, 0x38, 0x38));
  lv_obj_set_event_cb(btnPlayPause, btnEventHandler);
  lv_obj_set_size(btnPlayPause, LV_HOR_RES, 50);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);

  if (timerController.IsRunning()) {
    SetTimerRunning();
  } else {
    SetTimerStopped();
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

void Timer::SetTimerRunning() {
  lv_obj_set_hidden(minuteCounter.GetObject(), true);
  lv_obj_set_hidden(secondCounter.GetObject(), true);
  lv_label_set_text_static(txtPlayPause, "Pause");
}

void Timer::SetTimerStopped() {
  lv_obj_set_hidden(minuteCounter.GetObject(), false);
  lv_obj_set_hidden(secondCounter.GetObject(), false);
  lv_label_set_text_static(txtPlayPause, "Start");
}

void Timer::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnPlayPause) {
      if (timerController.IsRunning()) {
        uint32_t seconds = timerController.GetTimeRemaining() / 1000;
        minuteCounter.SetValue(seconds / 60);
        secondCounter.SetValue(seconds % 60);
        timerController.StopTimer();
        SetTimerStopped();
      } else if (secondCounter.GetValue() + minuteCounter.GetValue() > 0) {
        timerController.StartTimer((secondCounter.GetValue() + minuteCounter.GetValue() * 60) * 1000);
        Refresh();
        SetTimerRunning();
      }
    }
  }
}

void Timer::SetDone() {
  lv_label_set_text_static(time, "00:00");
  minuteCounter.SetValue(0);
  secondCounter.SetValue(0);
  SetTimerStopped();
}
