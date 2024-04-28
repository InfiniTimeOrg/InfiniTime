#include "displayapp/screens/Timer.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

static std::chrono::milliseconds lastTimerSetting;

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

static void counterChangeHandler(void *timerScreen) {
  Timer* timer = (Timer*)timerScreen;
  lastTimerSetting = timer->GetCounters();
}

Timer::Timer(Controllers::Timer& timerController) : timer {timerController} {

  lv_obj_t* colonLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(colonLabel, ":");
  lv_obj_align(colonLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, -29);

  minuteCounter.Create();
  secondCounter.Create();
  lv_obj_align(minuteCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_align(secondCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  minuteCounter.SetValueChangedEventCallback(this, counterChangeHandler);
  secondCounter.SetValueChangedEventCallback(this, counterChangeHandler);

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

  switch (timer.GetState()) {
    case Controllers::Timer::TimerState::Stopped:
      SetCounters(lastTimerSetting);
      SetInterfaceStopped();
      break;
    case Controllers::Timer::TimerState::Running:
      SetCounters(timer.GetTimeRemaining());
      SetInterfaceRunning();
      break;
    case Controllers::Timer::TimerState::Paused:
      SetCounters(timer.GetTimeRemaining());
      SetInterfacePaused();
      break;
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Timer::~Timer() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Timer::SetButtonText(const char* text) {
  lv_label_set_text_static(txtPlayPause, text);
  lv_obj_align(txtPlayPause, btnPlayPause, LV_ALIGN_CENTER, 0, 0);
}

void Timer::ButtonPressed() {
  pressTime = xTaskGetTickCount();
  buttonPressing = true;
}

void Timer::MaskReset() {
  buttonPressing = false;

  // A click event is processed before a release event,
  // so the release event would override the "Pause" text without this check
  switch (timer.GetState()) {
    case Controllers::Timer::TimerState::Stopped:
      SetButtonText("Start");
      break;
    case Controllers::Timer::TimerState::Running:
      SetButtonText("Pause");
      break;
    case Controllers::Timer::TimerState::Paused:
      SetButtonText("Resume");
      break;
  }

  maskPosition = 0;
  UpdateMask();
}

void Timer::HandleHold() {
  if (timer.GetState() == Controllers::Timer::TimerState::Stopped) {
    SetButtonText("Reset");
  } else {
    SetButtonText("Stop");
  }

  maskPosition += 15;
  if (maskPosition > 240) {
    HandleLongPress();
  } else {
    UpdateMask();
  }
}

void Timer::UpdateMask() {
  lv_draw_mask_line_param_t maskLine;

  lv_draw_mask_line_points_init(&maskLine, maskPosition, 0, maskPosition, 240, LV_DRAW_MASK_LINE_SIDE_LEFT);
  lv_objmask_update_mask(highlightObjectMask, highlightMask, &maskLine);

  lv_draw_mask_line_points_init(&maskLine, maskPosition, 0, maskPosition, 240, LV_DRAW_MASK_LINE_SIDE_RIGHT);
  lv_objmask_update_mask(btnObjectMask, btnMask, &maskLine);
}

void Timer::HandleLongPress() {
  if (timer.GetState() == Controllers::Timer::TimerState::Stopped) {
    Reset();
  } else {
    Stop();
  }

  MaskReset();
}

void Timer::Refresh() {
  if (timer.GetState() == Controllers::Timer::TimerState::Running) {
    auto secondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimeRemaining());
    SetCounters(secondsRemaining);
  } else {
    if (buttonPressing && xTaskGetTickCount() > pressTime + pdMS_TO_TICKS(150)) {
      HandleHold();
    }
  }
}

void Timer::SetInterfaceRunning() {
  minuteCounter.HideControls();
  secondCounter.HideControls();
  SetButtonText("Pause");
}

void Timer::SetInterfacePaused() {
  minuteCounter.HideControls();
  secondCounter.HideControls();
  SetButtonText("Resume");
}

void Timer::SetInterfaceStopped() {
  minuteCounter.ShowControls();
  secondCounter.ShowControls();
  SetButtonText("Start");
}

void Timer::SetCounters(const std::chrono::milliseconds& duration) {
  SetCounters(std::chrono::duration_cast<std::chrono::seconds>(duration));
}

void Timer::SetCounters(const std::chrono::seconds& duration) {
  minuteCounter.SetValue(duration.count() / 60);
  secondCounter.SetValue(duration.count() % 60);
}

std::chrono::seconds Timer::GetCounters() {
  return std::chrono::minutes(minuteCounter.GetValue()) + std::chrono::seconds(secondCounter.GetValue());
}

void Timer::ToggleRunning() {
  if (timer.GetState() == Controllers::Timer::TimerState::Stopped) {
    if (secondCounter.GetValue() + minuteCounter.GetValue() > 0) {
      timer.Start(GetCounters());
      Refresh();
      SetInterfaceRunning();
    }
  } else if (timer.GetState() == Controllers::Timer::TimerState::Running) {
    timer.Pause();
    SetInterfacePaused();
  } else { // Paused
    timer.Resume();
    SetInterfaceRunning();
  }
}

void Timer::Reset() {
  lastTimerSetting = std::chrono::seconds(0);
  Stop();
}

void Timer::Stop() {
  timer.Stop();
  SetCounters(lastTimerSetting);
  SetInterfaceStopped();
}
