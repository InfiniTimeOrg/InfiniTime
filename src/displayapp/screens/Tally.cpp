#include "displayapp/screens/Tally.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"
#include "components/motion/MotionController.h"
#include "components/motor/MotorController.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void countButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Tally*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->Increment();
    }
  }

  void resetButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Tally*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->Reset();
    }
  }

  void shakeButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Tally*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->ToggleShakeToCount();
    }
  }

  void awakeButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Tally*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->ToggleKeepAwake();
    }
  }
}

Tally::Tally(Controllers::MotionController& motionController,
             Controllers::MotorController& motorController,
             Controllers::Settings& settingsController,
             System::SystemTask& systemTask)
  : motionController {motionController}, motorController {motorController}, settingsController {settingsController}, wakeLock(systemTask) {

  // the count is actually a button (it was the easiest way to detect taps)
  countButton = lv_btn_create(lv_scr_act(), nullptr);
  countButton->user_data = this;
  lv_obj_set_event_cb(countButton, countButtonEventHandler);
  lv_obj_set_size(countButton, 240, 190);
  lv_obj_set_style_local_bg_color(countButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_align(countButton, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);
  countLabel = lv_label_create(countButton, nullptr);
  lv_obj_set_style_local_text_font(countLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &open_sans_light);

  resetButton = lv_btn_create(lv_scr_act(), nullptr);
  resetButton->user_data = this;
  lv_obj_set_event_cb(resetButton, resetButtonEventHandler);
  lv_obj_set_size(resetButton, 76, 50);
  lv_obj_align(resetButton, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  resetLabel = lv_label_create(resetButton, nullptr);
  lv_label_set_text_static(resetLabel, Symbols::undo);

  shakeButton = lv_btn_create(lv_scr_act(), nullptr);
  shakeButton->user_data = this;
  lv_obj_set_event_cb(shakeButton, shakeButtonEventHandler);
  lv_obj_set_size(shakeButton, 76, 50);
  lv_obj_set_style_local_bg_color(shakeButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_align(shakeButton, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  shakeLabel = lv_label_create(shakeButton, nullptr);
  lv_label_set_text_static(shakeLabel, Symbols::sort);

  awakeButton = lv_btn_create(lv_scr_act(), nullptr);
  awakeButton->user_data = this;
  lv_obj_set_event_cb(awakeButton, awakeButtonEventHandler);
  lv_obj_set_size(awakeButton, 76, 50);
  lv_obj_set_style_local_bg_color(awakeButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_align(awakeButton, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  awakeLabel = lv_label_create(awakeButton, nullptr);
  lv_label_set_text_static(awakeLabel, Symbols::eye);

  UpdateCount();

  refreshTask = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Tally::~Tally() {
  if (keepAwakeEnabled) {
    wakeLock.Release();
  }
  ShakeToWakeDisable();
  lv_task_del(refreshTask);
  lv_obj_clean(lv_scr_act());
}

void Tally::Refresh() {
  if (shakeToCountEnabled) {
    if (motionController.CurrentShakeSpeed() >= settingsController.GetShakeThreshold()) {
      if (shakeToCountDelay <= 0) {
        shakeToCountDelay = SHAKE_DELAY_TIME / LV_DISP_DEF_REFR_PERIOD;
        Increment();
      }
    } else if (shakeToCountDelay > 0) {
      shakeToCountDelay--;
    }
  }

  if (incrementDelay > 0) {
    incrementDelay--;
  }
}

void Tally::Increment() {
  if (incrementDelay <= 0) {
    incrementDelay = INCREMENT_DELAY_TIME / LV_DISP_DEF_REFR_PERIOD;
    count++;
    motorController.RunForDuration(80);
    UpdateCount();
  }
}

void Tally::Reset() {
  count = 0;
  UpdateCount();
}

void Tally::ToggleShakeToCount() {
  shakeToCountEnabled = !shakeToCountEnabled;
  if (shakeToCountEnabled) {
    ShakeToWakeEnable();
  } else {
    ShakeToWakeDisable();
  }
  lv_obj_set_style_local_bg_color(shakeButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, shakeToCountEnabled ? LV_COLOR_GREEN : LV_COLOR_RED);
}

void Tally::ToggleKeepAwake() {
  keepAwakeEnabled = !keepAwakeEnabled;
  if (keepAwakeEnabled) {
    wakeLock.Lock();
  } else {
    wakeLock.Release();
  }
  lv_obj_set_style_local_bg_color(awakeButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, keepAwakeEnabled ? LV_COLOR_GREEN : LV_COLOR_RED);
}

void Tally::UpdateCount() {
  lv_label_set_text_fmt(countLabel, "%d", count);
  lv_obj_realign(countLabel);
}

void Tally::ShakeToWakeEnable() {
  // if shake-to-wake is not enabled, enable it while this app is open
  shakeToWakeTempEnable = !settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::Shake);
  if (shakeToWakeTempEnable) {
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::Shake, true);
  }
}

void Tally::ShakeToWakeDisable() {
  // if shake-to-wake was not enabled before, disable it again
  if (shakeToWakeTempEnable) {
    settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::Shake, false);
    shakeToWakeTempEnable = false;
  }
}
