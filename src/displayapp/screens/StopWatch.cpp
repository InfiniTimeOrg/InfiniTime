#include "StopWatch.h"

#include "Screen.h"
#include "Symbols.h"
#include "lvgl/lvgl.h"
#include "projdefs.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include <tuple>

using namespace Pinetime::Applications::Screens;

// Anonymous namespace for local functions
namespace {
  TimeSeparated_t convertTicksToTimeSegments(const TickType_t timeElapsed) {
    const int timeElapsedMillis = (static_cast<float>(timeElapsed) / static_cast<float>(configTICK_RATE_HZ)) * 1000;

    const int hundredths = (timeElapsedMillis % 1000) / 10; // Get only the first two digits and ignore the last
    const int secs = (timeElapsedMillis / 1000) % 60;
    const int mins = (timeElapsedMillis / 1000) / 60;
    return TimeSeparated_t {mins, secs, hundredths};
  }

  TickType_t calculateDelta(const TickType_t startTime, const TickType_t currentTime) {
    TickType_t delta = 0;
    // Take care of overflow
    if (startTime > currentTime) {
      delta = 0xffffffff - startTime;
      delta += (currentTime + 1);
    } else {
      delta = currentTime - startTime;
    }
    return delta;
  }
}

static void play_pause_event_handler(lv_obj_t* obj, lv_event_t event) {
  auto stopWatch = static_cast<StopWatch*>(obj->user_data);
  stopWatch->playPauseBtnEventHandler(event);
}

static void stop_lap_event_handler(lv_obj_t* obj, lv_event_t event) {
  auto stopWatch = static_cast<StopWatch*>(obj->user_data);
  stopWatch->stopLapBtnEventHandler(event);
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
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text(time, "00:00");
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_CENTER, 0, -45);

  msecTime = lv_label_create(lv_scr_act(), nullptr);
  // lv_obj_set_style_local_text_font(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text(msecTime, "00");
  lv_obj_align(msecTime, lv_scr_act(), LV_ALIGN_CENTER, 0, 3);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, play_pause_event_handler);
  lv_obj_set_height(btnPlayPause, 50);
  lv_obj_set_width(btnPlayPause, 115);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  lv_label_set_text(txtPlayPause, Symbols::play);

  btnStopLap = lv_btn_create(lv_scr_act(), nullptr);
  btnStopLap->user_data = this;
  lv_obj_set_event_cb(btnStopLap, stop_lap_event_handler);
  lv_obj_set_height(btnStopLap, 50);
  lv_obj_set_width(btnStopLap, 115);
  lv_obj_align(btnStopLap, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x080808));
  txtStopLap = lv_label_create(btnStopLap, nullptr);
  lv_obj_set_style_local_text_color(txtStopLap, LV_BTN_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x888888));
  lv_label_set_text(txtStopLap, Symbols::stop);
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);

  lapOneText = lv_label_create(lv_scr_act(), nullptr);
  // lv_obj_set_style_local_text_font(lapOneText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(lapOneText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_align(lapOneText, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 50, 30);
  lv_label_set_text(lapOneText, "");

  lapTwoText = lv_label_create(lv_scr_act(), nullptr);
  // lv_obj_set_style_local_text_font(lapTwoText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(lapTwoText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_align(lapTwoText, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 50, 55);
  lv_label_set_text(lapTwoText, "");

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

StopWatch::~StopWatch() {
  lv_task_del(taskRefresh);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void StopWatch::reset() {
  currentState = States::Init;
  oldTimeElapsed = 0;
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  lv_label_set_text(time, "00:00");
  lv_label_set_text(msecTime, "00");

  lv_label_set_text(lapOneText, "");
  lv_label_set_text(lapTwoText, "");
  lapBuffer.clearBuffer();
  lapNr = 0;
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);
}

void StopWatch::start() {
  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
  lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_label_set_text(txtPlayPause, Symbols::pause);
  lv_label_set_text(txtStopLap, Symbols::lapsFlag);
  startTime = xTaskGetTickCount();
  currentState = States::Running;
  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

void StopWatch::pause() {
  startTime = 0;
  // Store the current time elapsed in cache
  oldTimeElapsed += timeElapsed;
  currentState = States::Halted;
  lv_label_set_text(txtPlayPause, Symbols::play);
  lv_label_set_text(txtStopLap, Symbols::stop);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void StopWatch::Refresh() {
  if (currentState == States::Running) {
    timeElapsed = calculateDelta(startTime, xTaskGetTickCount());
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
    start();
  } else if (currentState == States::Running) {
    pause();
  } else if (currentState == States::Halted) {
    start();
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
    reset();
  }
}

bool StopWatch::OnButtonPushed() {
  if (currentState == States::Running) {
    pause();
    return true;
  }
  return false;
}
