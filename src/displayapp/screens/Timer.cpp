#include "displayapp/screens/Timer.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;
using Pinetime::Controllers::TimerController;

#define TIMER_RING_DURATION_MSEC (10 * 1000)

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Timer*>(obj->user_data);
  if (event == LV_EVENT_PRESSED) {
    screen->ButtonPressed();
  } else if (event == LV_EVENT_RELEASED || event == LV_EVENT_PRESS_LOST) {
    screen->MaskReset();
  } else if (event == LV_EVENT_SHORT_CLICKED) {
    screen->ToggleRunning();
  }
}

static void StopRingingTaskCallback(lv_task_t* task) {
  auto* screen = static_cast<Timer*>(task->user_data);
  screen->StopRinging();
}

Timer::Timer(DisplayApp* app, TimerController& timerController, System::SystemTask& systemTask)
  : Screen(app), timerController {timerController}, systemTask {systemTask} {
  colonLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(colonLabel, ":");
  lv_obj_align(colonLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, -29);

  minuteCounter.Create();
  secondCounter.Create();
  lv_obj_align(minuteCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_align(secondCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  highlightObjectMask = lv_objmask_create(lv_scr_act(), nullptr);
  lv_obj_set_size(highlightObjectMask, 240, 50);
  lv_obj_align(highlightObjectMask, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  lv_draw_mask_line_param_t tmpMaskLine;

  lv_draw_mask_line_points_init(&tmpMaskLine, 0, 0, 0, 240, LV_DRAW_MASK_LINE_SIDE_LEFT);
  highlightMask = lv_objmask_add_mask(highlightObjectMask, &tmpMaskLine);

  lv_obj_t* btnHighlight = lv_obj_create(highlightObjectMask, nullptr);
  lv_obj_set_style_local_radius(btnHighlight, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(btnHighlight, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_obj_set_size(btnHighlight, LV_HOR_RES, 50);
  lv_obj_align(btnHighlight, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  btnObjectMask = lv_objmask_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btnObjectMask, 240, 50);
  lv_obj_align(btnObjectMask, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  lv_draw_mask_line_points_init(&tmpMaskLine, 0, 0, 0, 240, LV_DRAW_MASK_LINE_SIDE_RIGHT);
  btnMask = lv_objmask_add_mask(btnObjectMask, &tmpMaskLine);

  btnPlayPause = lv_btn_create(btnObjectMask, nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_style_local_radius(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_event_cb(btnPlayPause, btnEventHandler);
  lv_obj_set_size(btnPlayPause, LV_HOR_RES, 50);

  txtPlayPause = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(txtPlayPause, btnPlayPause, LV_ALIGN_CENTER, 0, 0);

  switch (timerController.State()) {
    case TimerController::TimerState::Running:
      SetTimerRunning();
      break;
    case TimerController::TimerState::Stopped:
      SetTimerStopped();
      break;
    case TimerController::TimerState::Finished:
      SetTimerFinished();
      break;
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Timer::~Timer() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Timer::ButtonPressed() {
  pressTime = xTaskGetTickCount();
  buttonPressing = true;
}

void Timer::MaskReset() {
  buttonPressing = false;
  // A click event is processed before a release event,
  // so the release event would override the text without this check
  if (timerController.State() == TimerController::TimerState::Stopped) {
    lv_label_set_text_static(txtPlayPause, "Start");
  } else if (timerController.State() == TimerController::TimerState::Finished) {
    lv_label_set_text_static(txtPlayPause, "Stop");
  }
  maskPosition = 0;
  UpdateMask();
}

void Timer::UpdateMask() {
  lv_draw_mask_line_param_t maskLine;

  lv_draw_mask_line_points_init(&maskLine, maskPosition, 0, maskPosition, 240, LV_DRAW_MASK_LINE_SIDE_LEFT);
  lv_objmask_update_mask(highlightObjectMask, highlightMask, &maskLine);

  lv_draw_mask_line_points_init(&maskLine, maskPosition, 0, maskPosition, 240, LV_DRAW_MASK_LINE_SIDE_RIGHT);
  lv_objmask_update_mask(btnObjectMask, btnMask, &maskLine);
}

void Timer::Refresh() {
  if (timerController.State() != TimerController::TimerState::Stopped) {
    // Update counters if running or finished
    uint32_t seconds = timerController.GetTimeRemainingMs() / 1000;
    minuteCounter.SetValue(seconds / 60);
    secondCounter.SetValue(seconds % 60);
  }
  if (timerController.State() != TimerController::TimerState::Running && buttonPressing &&
      xTaskGetTickCount() > pressTime + pdMS_TO_TICKS(150)) {
    // Support long-pressing the button if stopped or finished
    lv_label_set_text_static(txtPlayPause, "Reset");
    maskPosition += 15;
    if (maskPosition > 240) {
      MaskReset();
      if (timerController.State() == TimerController::TimerState::Stopped) {
        Reset();
      } else {
        StopRinging();
        timerController.StopTimer();
        systemTask.PushMessage(System::Messages::GoToClock);
      }
    } else {
      UpdateMask();
    }
  }
}

void Timer::UpdateColor() {
  lv_color_t color = timerController.State() == TimerController::TimerState::Finished ? LV_COLOR_RED : LV_COLOR_WHITE;
  lv_obj_set_style_local_text_color(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
  minuteCounter.SetTextColor(color);
  secondCounter.SetTextColor(color);
}

void Timer::SetTimerRunning() {
  minuteCounter.HideControls();
  secondCounter.HideControls();
  lv_label_set_text_static(txtPlayPause, "Pause");
  MaskReset();
  UpdateColor();
}

void Timer::SetTimerFinished() {
  minuteCounter.HideControls();
  secondCounter.HideControls();
  lv_label_set_text_static(txtPlayPause, "Stop");
  UpdateColor();
  uint32_t msecRemaining = timerController.GetTimeRemainingMs();
  if (msecRemaining < TIMER_RING_DURATION_MSEC) {
    taskStopRinging =
      lv_task_create(StopRingingTaskCallback, pdMS_TO_TICKS(TIMER_RING_DURATION_MSEC - msecRemaining), LV_TASK_PRIO_MID, this);
    systemTask.PushMessage(System::Messages::DisableSleeping);
  }
}

bool Timer::StopRinging() {
  if (taskStopRinging == nullptr) {
    return false;
  }
  lv_task_del(taskStopRinging);
  taskStopRinging = nullptr;
  systemTask.PushMessage(System::Messages::StopRinging);
  systemTask.PushMessage(System::Messages::EnableSleeping);
  return true;
}

void Timer::SetTimerStopped() {
  minuteCounter.ShowControls();
  secondCounter.ShowControls();
  lv_label_set_text_static(txtPlayPause, "Start");
  UpdateColor();
}

void Timer::ToggleRunning() {
  switch (timerController.State()) {
    case TimerController::TimerState::Running: {
      uint32_t seconds = timerController.GetTimeRemainingMs() / 1000;
      minuteCounter.SetValue(seconds / 60);
      secondCounter.SetValue(seconds % 60);
    }
      timerController.StopTimer();
      SetTimerStopped();
      break;
    case TimerController::TimerState::Finished:
      StopRinging();
      timerController.StopTimer();
      Reset();
      break;
    case TimerController::TimerState::Stopped:
      if (secondCounter.GetValue() + minuteCounter.GetValue() > 0) {
        timerController.StartTimer((secondCounter.GetValue() + minuteCounter.GetValue() * 60) * 1000);
        Refresh();
        SetTimerRunning();
      }
      break;
  }
}

void Timer::Reset() {
  minuteCounter.SetValue(0);
  secondCounter.SetValue(0);
  SetTimerStopped();
}

bool Timer::OnButtonPushed() {
  return StopRinging();
}

bool Timer::OnTouchEvent(TouchEvents event) {
  if (event == TouchEvents::SwipeDown) {
    StopRinging();
  }
  return false;
}