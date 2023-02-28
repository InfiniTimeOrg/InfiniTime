#include "displayapp/screens/Timer.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;
using Pinetime::Controllers::TimerController;

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Timer*>(obj->user_data);
  if (event == LV_EVENT_PRESSED) {
    screen->ButtonPressed(obj);
  } else if (event == LV_EVENT_RELEASED || event == LV_EVENT_PRESS_LOST) {
    screen->ButtonReleased(obj);
  } else if (event == LV_EVENT_SHORT_CLICKED) {
    screen->ButtonShortClicked(obj);
  }
}

static void SnoozeAlertTaskCallback(lv_task_t* task) {
  auto* screen = static_cast<Timer*>(task->user_data);
  screen->SnoozeAlert();
}

Timer::Timer(Controllers::TimerController& timerController,
             System::SystemTask& systemTask,
             Controllers::MotorController& motorController)
  : timerController {timerController}, systemTask {systemTask}, motorController {motorController} {

  lv_obj_t* colonLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(colonLabel, ":");
  lv_obj_align(colonLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, -29);

  hourCounter.Create();
  minuteCounter.Create();
  
  lv_obj_align(hourCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_align(minuteCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

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

  txtPlayPause = lv_label_create(btnPlayPause, nullptr);

  btnStop = lv_btn_create(lv_scr_act(), nullptr);
  btnStop->user_data = this;
  lv_obj_set_event_cb(btnStop, btnEventHandler);
  lv_obj_set_size(btnStop, 115, 50);
  lv_obj_align(btnStop, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(btnStop, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_set_hidden(btnStop, true);
  
  txtStop = lv_label_create(btnStop, nullptr);
  lv_label_set_text_static(txtStop, Symbols::stop);

  btnSnooze = lv_btn_create(lv_scr_act(), nullptr);
  btnSnooze->user_data = this;
  lv_obj_set_event_cb(btnSnooze, btnEventHandler);
  lv_obj_set_size(btnSnooze, 115, 50);
  lv_obj_align(btnSnooze, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lv_obj_set_style_local_bg_color(btnSnooze, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);
  lv_obj_set_hidden(btnSnooze, true);
  
  txtSnooze = lv_label_create(btnSnooze, nullptr);

  switch (timerController.State()){
    case TimerController::TimerState::Dormant:
      Reset();
      break;
    case TimerController::TimerState::Running:
      ShowTimerRunning();
      break;
    case TimerController::TimerState::Alerting:
      SetAlerting();
      break;
    case TimerController::TimerState::Snoozed:
      ShowSnoozed();
      break;
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Timer::~Timer() {
  if (timerController.State() == TimerController::TimerState::Alerting) {
    StopAlerting();
  }
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Timer::ButtonPressed(lv_obj_t* obj) {
  if(obj == btnPlayPause) {
    pressTime = xTaskGetTickCount();
    buttonPressing = true;
    return;
  }
}

void Timer::ButtonReleased(lv_obj_t* obj) {
  if(obj == btnPlayPause){
    MaskReset();
    return;
  }
}

void Timer::ButtonShortClicked(lv_obj_t* obj) {
  if(obj == btnPlayPause){
    ToggleRunning();
    return;
  }
  if(obj == btnSnooze){
    SnoozeAlert();
    return;
  }
  if(obj == btnStop){
    StopAlerting();
    return;
  }
}

// Pysical side button
bool Timer::OnButtonPushed() {
  if (timerController.State() == TimerController::TimerState::Alerting) {
    SnoozeAlert();
    return true;
  }
  return false;
}

bool Timer::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  // Don't allow closing the screen by swiping while the timer is alerting
  return event == TouchEvents::SwipeDown && timerController.State() == TimerController::TimerState::Alerting;
}

void Timer::MaskReset() {
  buttonPressing = false;
  // A click event is processed before a release event,
  // so the release event would override the "Pause" text without this check
  if (!timerController.IsRunning()) {
    lv_label_set_text_static(txtPlayPause, "Start");
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
  if (timerController.IsRunning()) {
    auto minutesRemaining = std::chrono::duration_cast<std::chrono::minutes>(timerController.GetTimeRemaining());
    auto hoursRemaining = std::chrono::duration_cast<std::chrono::hours>(minutesRemaining);
    hourCounter.SetValue(hoursRemaining.count());
    minuteCounter.SetValue(minutesRemaining.count() % 60);
  } else if (buttonPressing && xTaskGetTickCount() > pressTime + pdMS_TO_TICKS(150)) {
    lv_label_set_text_static(txtPlayPause, "Reset");
    maskPosition += 15;
    if (maskPosition > 240) {
      MaskReset();
      Reset();
    } else {
      UpdateMask();
    }
  }
}

void Timer::ShowTimerRunning() {
  hourCounter.HideControls();
  minuteCounter.HideControls();
  lv_label_set_text_static(txtPlayPause, "Pause");
}

void Timer::ShowTimerStopped() {
  hourCounter.ShowControls();
  minuteCounter.ShowControls();
  lv_label_set_text_static(txtPlayPause, "Start");
}

void Timer::ToggleRunning() {
  if (timerController.IsRunning()) {
    auto minutesRemaining = std::chrono::duration_cast<std::chrono::minutes>(timerController.GetTimeRemaining());
    auto hoursRemaining = std::chrono::duration_cast<std::chrono::hours>(minutesRemaining);
    hourCounter.SetValue(hoursRemaining.count());
    minuteCounter.SetValue(minutesRemaining.count() % 60);
    timerController.StopTimer();
    ShowTimerStopped();
  } else if (hourCounter.GetValue() + minuteCounter.GetValue() > 0) {
    timerController.StartTimer(std::chrono::hours(hourCounter.GetValue()), std::chrono::minutes(minuteCounter.GetValue()));
    Refresh();
    ShowTimerRunning();
  }
}

void Timer::Reset() {
  hourCounter.SetValue(timerController.Hours().count());
  minuteCounter.SetValue(timerController.Minutes().count());
  ShowTimerStopped();
}

void Timer::ShowAlertingButtons() {
  ShowTimerRunning();
  lv_obj_set_hidden(btnPlayPause, true);
  lv_obj_set_hidden(btnStop, false);
  lv_obj_set_hidden(btnSnooze, false);
}

void Timer::ShowAlerting() {
  ShowAlertingButtons();
  lv_label_set_text_static(txtSnooze, "zZ");
  lv_obj_set_state(btnSnooze, LV_STATE_DEFAULT);
  lv_obj_set_state(txtSnooze, LV_STATE_DEFAULT);
}

void Timer::SetAlerting() {
  ShowAlerting();

  taskStopAlert = lv_task_create(SnoozeAlertTaskCallback, pdMS_TO_TICKS(alertTime.count()), LV_TASK_PRIO_MID, this);
  motorController.StartRinging();
  systemTask.PushMessage(System::Messages::DisableSleeping);
}

void Timer::ShowSnoozed(){
  ShowAlertingButtons();

  lv_label_set_text_static(txtSnooze, "Snoozed");
  lv_obj_set_state(btnSnooze, LV_STATE_DISABLED);
  lv_obj_set_state(txtSnooze, LV_STATE_DISABLED);
}

void Timer::SnoozeAlert() {
  ShowSnoozed();

  timerController.SnoozeAlert();
  motorController.StopRinging();

  if (taskStopAlert != nullptr) {
    lv_task_del(taskStopAlert);
    taskStopAlert = nullptr;
  }
  systemTask.PushMessage(System::Messages::EnableSleeping);
}

void Timer::StopAlerting() {
  timerController.StopAlerting();
  motorController.StopRinging();
  if (taskStopAlert != nullptr) {
    lv_task_del(taskStopAlert);
    taskStopAlert = nullptr;
  }
  systemTask.PushMessage(System::Messages::EnableSleeping);
  lv_obj_set_hidden(btnStop, true);
  lv_obj_set_hidden(btnSnooze, true);
  lv_obj_set_hidden(btnPlayPause, false);
  Reset();
}
