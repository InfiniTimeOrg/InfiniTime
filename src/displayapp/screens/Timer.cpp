#include "displayapp/screens/Timer.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

// Initialize static member with default timer durations (5min, 10min, 15min)
uint32_t Timer::timerDurations[Timer::numRecentTimers] = {300000, 600000, 900000};

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Timer*>(obj->user_data);
  if (screen->launcherMode && event == LV_EVENT_CLICKED) {
    screen->OnLauncherButtonClicked(obj);
  } else if (event == LV_EVENT_PRESSED) {
    screen->ButtonPressed();
  } else if (event == LV_EVENT_RELEASED || event == LV_EVENT_PRESS_LOST) {
    screen->MaskReset();
  } else if (event == LV_EVENT_SHORT_CLICKED) {
    screen->ToggleRunning();
  }
}

Timer::Timer(Controllers::Timer& timerController, Controllers::MotorController& motorController)
  : timer {timerController}, motorController {motorController} {

  // If timer is already running, skip launcher and go directly to timer UI
  if (timer.IsRunning()) {
    uint32_t durationMs = GetTimerDuration(0);
    CreateTimerUI(durationMs, false);
  } else {
    CreateLauncherUI();
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Timer::~Timer() {
  lv_task_del(taskRefresh);
  if (launcherMode) {
    lv_style_reset(&btnStyle);
  }
  lv_obj_clean(lv_scr_act());
}

void Timer::ButtonPressed() {
  pressTime = xTaskGetTickCount();
  buttonPressing = true;
}

void Timer::MaskReset() {
  buttonPressing = false;
  // A click event is processed before a release event,
  // so the release event would override the "Pause" text without this check
  if (!timer.IsRunning()) {
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
  // Don't try to update timer display if we're in launcher mode (counters don't exist)
  if (launcherMode) {
    // If timer starts while in launcher, transition to timer UI
    if (timer.IsRunning()) {
      uint32_t durationMs = GetTimerDuration(0);
      lv_style_reset(&btnStyle);
      lv_obj_clean(lv_scr_act());
      CreateTimerUI(durationMs, false);
    }
    return;
  }

  if (timer.IsRunning()) {
    DisplayTime();
  } else if (buttonPressing && xTaskGetTickCount() - pressTime > pdMS_TO_TICKS(150)) {
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

void Timer::DisplayTime() {
  displaySeconds =
    std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimerState().value_or(Controllers::Timer::TimerStatus {}).distanceToExpiry);
  if (displaySeconds.IsUpdated()) {
    minuteCounter.SetValue(displaySeconds.Get().count() / 60);
    secondCounter.SetValue(displaySeconds.Get().count() % 60);
  }
}

void Timer::SetTimerRunning() {
  if (launcherMode) {
    return;
  }
  minuteCounter.HideControls();
  secondCounter.HideControls();
  lv_label_set_text_static(txtPlayPause, "Pause");
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
}

void Timer::SetTimerStopped() {
  if (launcherMode) {
    return;
  }
  minuteCounter.ShowControls();
  secondCounter.ShowControls();
  lv_label_set_text_static(txtPlayPause, "Start");
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
}

void Timer::SetTimerRinging() {
  if (launcherMode) {
    // Timer expired while in launcher mode - transition will happen in Refresh()
    return;
  }
  minuteCounter.HideControls();
  secondCounter.HideControls();
  lv_label_set_text_static(txtPlayPause, "Reset");
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
}

void Timer::ToggleRunning() {
  if (timer.IsRunning()) {
    DisplayTime();
    timer.StopTimer();
    SetTimerStopped();
  } else if (secondCounter.GetValue() + minuteCounter.GetValue() > 0) {
    auto timerDuration = std::chrono::minutes(minuteCounter.GetValue()) + std::chrono::seconds(secondCounter.GetValue());
    timer.StartTimer(timerDuration);

    // Add the timer duration to MRU list
    uint32_t durationMs = (minuteCounter.GetValue() * 60 + secondCounter.GetValue()) * 1000;
    AddTimerDuration(durationMs);

    Refresh();
    SetTimerRunning();
  }
}

void Timer::Reset() {
  DisplayTime();
  SetTimerStopped();
}

void Timer::AddTimerDuration(uint32_t duration) {
  // If already at front, nothing to do
  if (duration == timerDurations[0]) {
    return;
  }

  // Shift elements down, stopping after we find the duration
  uint32_t prev = timerDurations[0];
  for (int i = 1; i < numRecentTimers; i++) {
    uint32_t temp = timerDurations[i];
    timerDurations[i] = prev;
    prev = temp;
    if (temp == duration) {
      // Found it - stop after this shift
      break;
    }
  }

  // Insert duration at front
  timerDurations[0] = duration;
}

uint32_t Timer::GetTimerDuration(uint8_t index) const {
  if (index >= numRecentTimers) {
    return timerDurations[0];
  }
  return timerDurations[index];
}

void Timer::CreateLauncherUI() {
  static constexpr uint8_t innerDistance = 10;
  static constexpr uint8_t buttonHeight = (LV_VER_RES_MAX - innerDistance) / 2;
  static constexpr uint8_t buttonWidth = (LV_HOR_RES_MAX - innerDistance) / 2;

  lv_style_init(&btnStyle);
  lv_style_set_radius(&btnStyle, LV_STATE_DEFAULT, buttonHeight / 4);
  lv_style_set_bg_color(&btnStyle, LV_STATE_DEFAULT, Colors::bgAlt);

  // Layout positions for the 3 recent timer buttons
  static constexpr lv_align_t buttonAlignments[numRecentTimers] = {
    LV_ALIGN_IN_TOP_LEFT,   // Button 0: Top-left
    LV_ALIGN_IN_TOP_RIGHT,  // Button 1: Top-right
    LV_ALIGN_IN_BOTTOM_LEFT // Button 2: Bottom-left
  };

  // Create each of the recent timer buttons
  for (int i = 0; i < numRecentTimers; i++) {
    btnRecent[i] = lv_btn_create(lv_scr_act(), nullptr);
    btnRecent[i]->user_data = this;
    lv_obj_set_event_cb(btnRecent[i], btnEventHandler);
    lv_obj_add_style(btnRecent[i], LV_BTN_PART_MAIN, &btnStyle);
    lv_obj_set_size(btnRecent[i], buttonWidth, buttonHeight);
    lv_obj_align(btnRecent[i], nullptr, buttonAlignments[i], 0, 0);

    uint32_t duration = GetTimerDuration(i);
    uint32_t minutes = duration / 60000;
    uint32_t seconds = (duration % 60000) / 1000;

    labelRecent[i] = lv_label_create(btnRecent[i], nullptr);
    lv_obj_t* labelIcon = lv_label_create(btnRecent[i], nullptr);

    // Show the minutes
    lv_obj_set_style_local_text_font(labelRecent[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
    lv_label_set_text_fmt(labelRecent[i], "%lu", minutes);
    lv_obj_align(labelRecent[i], btnRecent[i], LV_ALIGN_CENTER, 0, -20);

    // Show the seconds, or "min" below
    lv_obj_set_style_local_text_font(labelIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
    if (seconds == 0) {
      lv_label_set_text_static(labelIcon, "min");
    } else {
      lv_label_set_text_fmt(labelIcon, ":%02lu", seconds);
    }
    lv_obj_align(labelIcon, btnRecent[i], LV_ALIGN_CENTER, 0, 20);
  }

  // Bottom-right: New timer
  btnCustom = lv_btn_create(lv_scr_act(), nullptr);
  btnCustom->user_data = this;
  lv_obj_set_event_cb(btnCustom, btnEventHandler);
  lv_obj_add_style(btnCustom, LV_BTN_PART_MAIN, &btnStyle);
  lv_obj_set_size(btnCustom, buttonWidth, buttonHeight);
  lv_obj_align(btnCustom, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  labelCustom = lv_label_create(btnCustom, nullptr);
  lv_obj_set_style_local_text_font(labelCustom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_label_set_text_static(labelCustom, "+");
}

void Timer::CreateTimerUI(uint32_t startDurationMs, bool autoStart) {
  launcherMode = false;

  lv_obj_t* colonLabel = lv_label_create(lv_scr_act(), nullptr);
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

  // Create the label as a child of the button so it stays centered by default
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);

  // Reset button press state
  buttonPressing = false;
  pressTime = 0;

  if (timer.IsRunning()) {
    SetTimerRunning();
    DisplayTime();
  } else if (autoStart) {
    auto timerDuration = std::chrono::milliseconds(startDurationMs);
    timer.StartTimer(timerDuration);
    AddTimerDuration(startDurationMs);
    SetTimerRunning();
    DisplayTime();
  } else {
    // Set the initial duration only when timer is stopped
    uint32_t minutes = startDurationMs / 60000;
    uint32_t seconds = (startDurationMs % 60000) / 1000;
    minuteCounter.SetValue(minutes);
    secondCounter.SetValue(seconds);
    SetTimerStopped();
  }
}

void Timer::OnLauncherButtonClicked(lv_obj_t* obj) {
  uint32_t durationMs;
  bool autoStart;

  // Check if it's one of the recent timer buttons
  bool found = false;
  for (int i = 0; i < numRecentTimers; i++) {
    if (obj == btnRecent[i]) {
      durationMs = GetTimerDuration(i);
      autoStart = true;
      found = true;
      break;
    }
  }

  // Check if it's the custom timer button
  if (!found) {
    if (obj == btnCustom) {
      durationMs = 0;
      autoStart = false;
    } else {
      return;
    }
  }

  lv_style_reset(&btnStyle);
  lv_obj_clean(lv_scr_act());

  CreateTimerUI(durationMs, autoStart);

  // Wait for button release to prevent the press state from carrying over to the new UI
  lv_indev_wait_release(lv_indev_get_act());
}
