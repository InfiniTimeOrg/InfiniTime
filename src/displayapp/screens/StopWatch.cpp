#include "displayapp/screens/StopWatch.h"

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <lvgl/lvgl.h>
#include <FreeRTOS.h>
#include <task.h>

using namespace Pinetime::Applications::Screens;

namespace {
  TimeSeparated_t convertTicksToTimeSegments(const TickType_t timeElapsed) {
    // Centiseconds
    const int timeElapsedCentis = timeElapsed * 100 / configTICK_RATE_HZ;

    const int hundredths = (timeElapsedCentis % 100);
    const int secs = (timeElapsedCentis / 100) % 60;
    const int mins = (timeElapsedCentis / 100) / 60;
    return TimeSeparated_t {mins, secs, hundredths};
  }

  void play_pause_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    stopWatch->playPauseBtnEventHandler(event);
  }

  void stop_lap_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    stopWatch->stopLapBtnEventHandler(event);
  }
}

StopWatch::StopWatch(DisplayApp* app, System::SystemTask& systemTask)
  : Screen(app),
    systemTask {systemTask},
    currentState {States::Init},
    startTime {},
    oldTimeElapsed {},
    currentTimeSeparated {},
    lapBuffer {},
    lapNr {} {

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb0, 0xb0, 0xb0));
  lv_label_set_text_static(time, "00:00");
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_CENTER, 0, -45);

  msecTime = lv_label_create(lv_scr_act(), nullptr);
  // lv_obj_set_style_local_text_font(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb0, 0xb0, 0xb0));
  lv_label_set_text_static(msecTime, "00");
  lv_obj_align(msecTime, lv_scr_act(), LV_ALIGN_CENTER, 0, 3);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, play_pause_event_handler);
  lv_obj_set_size(btnPlayPause, 115, 50);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  lv_label_set_text_static(txtPlayPause, Symbols::play);

  btnStopLap = lv_btn_create(lv_scr_act(), nullptr);
  btnStopLap->user_data = this;
  lv_obj_set_event_cb(btnStopLap, stop_lap_event_handler);
  lv_obj_set_size(btnStopLap, 115, 50);
  lv_obj_align(btnStopLap, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DISABLED, LV_COLOR_MAKE(0x18, 0x18, 0x18));
  txtStopLap = lv_label_create(btnStopLap, nullptr);
  lv_obj_set_style_local_text_color(txtStopLap, LV_BTN_PART_MAIN, LV_STATE_DISABLED, LV_COLOR_MAKE(0xb0, 0xb0, 0xb0));
  lv_label_set_text_static(txtStopLap, Symbols::stop);
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);

  lapOneText = lv_label_create(lv_scr_act(), nullptr);
  // lv_obj_set_style_local_text_font(lapOneText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(lapOneText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_align(lapOneText, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 50, 30);
  lv_label_set_text_static(lapOneText, "");

  lapTwoText = lv_label_create(lv_scr_act(), nullptr);
  // lv_obj_set_style_local_text_font(lapTwoText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(lapTwoText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_align(lapTwoText, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 50, 55);
  lv_label_set_text_static(lapTwoText, "");

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

StopWatch::~StopWatch() {
  lv_task_del(taskRefresh);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void StopWatch::Reset() {
  currentState = States::Init;
  oldTimeElapsed = 0;
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb0, 0xb0, 0xb0));
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb0, 0xb0, 0xb0));

  lv_label_set_text_static(time, "00:00");
  lv_label_set_text_static(msecTime, "00");

  lv_label_set_text_static(lapOneText, "");
  lv_label_set_text_static(lapTwoText, "");
  lapBuffer.clearBuffer();
  lapNr = 0;
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);
}

void StopWatch::Start() {
  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
  lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x0, 0xb0, 0x0));
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x0, 0xb0, 0x0));
  lv_label_set_text_static(txtPlayPause, Symbols::pause);
  lv_label_set_text_static(txtStopLap, Symbols::lapsFlag);
  startTime = xTaskGetTickCount();
  currentState = States::Running;
  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

void StopWatch::Pause() {
  startTime = 0;
  // Store the current time elapsed in cache
  oldTimeElapsed += timeElapsed;
  currentState = States::Halted;
  lv_label_set_text_static(txtPlayPause, Symbols::play);
  lv_label_set_text_static(txtStopLap, Symbols::stop);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void StopWatch::Refresh() {
  if (currentState == States::Running) {
    timeElapsed = xTaskGetTickCount() - startTime;
    currentTimeSeparated = convertTicksToTimeSegments((oldTimeElapsed + timeElapsed));

    lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
    lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.hundredths);
  }
}

void StopWatch::playPauseBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  if (currentState == States::Init) {
    Start();
  } else if (currentState == States::Running) {
    Pause();
  } else if (currentState == States::Halted) {
    Start();
  }
}

void StopWatch::stopLapBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  // If running, then this button is used to save laps
  if (currentState == States::Running) {
    lapBuffer.addLaps(currentTimeSeparated);
    lapNr++;
    if (lapBuffer[1]) {
      lv_label_set_text_fmt(
        lapOneText, "#%2d   %2d:%02d.%02d", (lapNr - 1), lapBuffer[1]->mins, lapBuffer[1]->secs, lapBuffer[1]->hundredths);
    }
    if (lapBuffer[0]) {
      lv_label_set_text_fmt(lapTwoText, "#%2d   %2d:%02d.%02d", lapNr, lapBuffer[0]->mins, lapBuffer[0]->secs, lapBuffer[0]->hundredths);
    }
  } else if (currentState == States::Halted) {
    Reset();
  }
}

bool StopWatch::OnButtonPushed() {
  if (currentState == States::Running) {
    Pause();
    return true;
  }
  return false;
}
