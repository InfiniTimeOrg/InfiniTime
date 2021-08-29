#include "Timer.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void PlayPauseEventCallback(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Timer*>(obj->user_data);
    screen->PlayPauseEventHandler(obj, event);
  }

  void AdjustButtonEventCallback(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Timer*>(obj->user_data);
    screen->AdjustButtonEventHandler(obj, event);
  }
}

Timer::Timer(DisplayApp* app, Controllers::TimerController& timerController) : Screen(app), timerController {timerController} {

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text_static(time, "00:00");
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_CENTER, 0, -20);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, PlayPauseEventCallback);
  lv_obj_set_size(btnPlayPause, 120, 40);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  for (uint8_t i = 0; i < 4; i++) {
    buttons[i] = lv_btn_create(lv_scr_act(), nullptr);
    buttons[i]->user_data = this;
    lv_obj_set_event_cb(buttons[i], AdjustButtonEventCallback);
    lv_obj_set_size(buttons[i], 60, 40);

    static constexpr uint8_t extraArea = 10;
    lv_obj_set_ext_click_area(buttons[i], extraArea, extraArea, extraArea, extraArea);
  }

  lv_obj_align(buttons[minUp], lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 20, -80);
  lv_obj_set_style_local_value_str(buttons[minUp], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");

  lv_obj_align(buttons[minDown], lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 20, 40);
  lv_obj_set_style_local_value_str(buttons[minDown], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");

  lv_obj_align(buttons[secUp], lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -20, -80);
  lv_obj_set_style_local_value_str(buttons[secUp], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");

  lv_obj_align(buttons[secDown], lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -20, 40);
  lv_obj_set_style_local_value_str(buttons[secDown], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");

  if (timerController.IsRunning()) {
    SetButtonsHidden(true);
    lv_obj_set_style_local_value_str(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::pause);
  } else {
    lv_obj_set_style_local_value_str(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::play);
  }
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  Refresh();
}

Timer::~Timer() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Timer::UpdateTime() {
  lv_label_set_text_fmt(time, "%02d:%02d", minutes, seconds);
}

void Timer::Refresh() {
  if (timerController.IsRunning()) {
    remainingTime = timerController.GetTimeRemaining() / 1000 + 1;
    if (remainingTime.IsUpdated()) {
      minutes = remainingTime.Get() / 60;
      seconds = remainingTime.Get() % 60;
      UpdateTime();
    }
  }
}

void Timer::SetButtonsHidden(bool hidden) {
  for (uint8_t i = 0; i < 4; i++) {
    lv_obj_set_hidden(buttons[i], hidden);
  }
}

void Timer::AdjustButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED && event != LV_EVENT_LONG_PRESSED_REPEAT) {
    return;
  }

  if (obj == buttons[minUp]) {
    minutes = (minutes + 1) % 61;
  } else if (obj == buttons[minDown]) {
    if (minutes == 0) {
      minutes = 60;
    } else {
      minutes--;
    }
  } else if (obj == buttons[secUp]) {
    seconds = (seconds + 1) % 60;
  } else if (obj == buttons[secDown]) {
    if (seconds == 0) {
      seconds = 59;
    } else {
      seconds--;
    }
  }

  UpdateTime();
}

void Timer::PlayPauseEventHandler(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }

  if (timerController.IsRunning()) {
    lv_obj_set_style_local_value_str(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::play);
    timerController.StopTimer();
    SetButtonsHidden(false);
  } else if (seconds + minutes > 0) {
    lv_obj_set_style_local_value_str(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::pause);
    timerController.StartTimer((seconds + minutes * 60) * 1000);
    SetButtonsHidden(true);
  }
}

void Timer::SetDone() {
  lv_obj_set_style_local_value_str(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::play);
  seconds = 0;
  minutes = 0;
  UpdateTime();
  SetButtonsHidden(false);
}
