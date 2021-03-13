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

    const int milliSecs = (timeElapsedMillis % 1000) / 10; // Get only the first two digits and ignore the last
    const int secs = (timeElapsedMillis / 1000) % 60;
    const int mins = (timeElapsedMillis / 1000) / 60;
    return TimeSeparated_t {mins, secs, milliSecs};
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
  StopWatch* stopWatch = static_cast<StopWatch*>(obj->user_data);
  stopWatch->playPauseBtnEventHandler(event);
}

static void stop_lap_event_handler(lv_obj_t* obj, lv_event_t event) {
  StopWatch* stopWatch = static_cast<StopWatch*>(obj->user_data);
  stopWatch->stopLapBtnEventHandler(event);
}

StopWatch::StopWatch(DisplayApp* app, const Pinetime::Controllers::DateTime& dateTime)
  : Screen(app), dateTime {dateTime}, running {true}, currentState {States::INIT}, currentEvent {Events::STOP}, startTime {},
    oldTimeElapsed {}, currentTimeSeparated {}, lapBuffer {}, lapNr {}, lapPressed {false} {

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -50);
  lv_label_set_text(time, "00:00");

  msecTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_align(msecTime, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 113, 0);
  lv_label_set_text(msecTime, "00");

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, play_pause_event_handler);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  lv_label_set_text(txtPlayPause, Symbols::play);

  lapOneText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lapOneText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_align(lapOneText, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 50, 25);
  lv_label_set_text(lapOneText, "");

  lapTwoText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lapTwoText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_align(lapTwoText, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 50, 50);
  lv_label_set_text(lapTwoText, "");

  // We don't want this button in the init state
  btnStopLap = nullptr;
}

StopWatch::~StopWatch() {
  lv_obj_clean(lv_scr_act());
}

bool StopWatch::Refresh() {

  switch (currentState) {
    // Init state when an user first opens the app
    // and when a stop/reset button is pressed
    case States::INIT: {
      if (btnStopLap) {
        lv_obj_del(btnStopLap);
      }
      // The initial default value
      lv_label_set_text(time, "00:00");
      lv_label_set_text(msecTime, "00");

      lv_label_set_text(lapOneText, "");
      lv_label_set_text(lapTwoText, "");
      lapBuffer.clearBuffer();
      lapNr = 0;

      if (currentEvent == Events::PLAY) {
        btnStopLap = lv_btn_create(lv_scr_act(), nullptr);
        btnStopLap->user_data = this;
        lv_obj_set_event_cb(btnStopLap, stop_lap_event_handler);
        lv_obj_align(btnStopLap, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
        txtStopLap = lv_label_create(btnStopLap, nullptr);
        lv_label_set_text(txtStopLap, Symbols::lapsFlag);

        startTime = xTaskGetTickCount();
        currentState = States::RUNNING;
      }
      break;
    }
    case States::RUNNING: {
      lv_label_set_text(txtPlayPause, Symbols::pause);
      lv_label_set_text(txtStopLap, Symbols::lapsFlag);

      const auto timeElapsed = calculateDelta(startTime, xTaskGetTickCount());
      currentTimeSeparated = convertTicksToTimeSegments((oldTimeElapsed + timeElapsed));

      lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
      lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.msecs);

      if (lapPressed == true) {
        if (lapBuffer[1]) {
          lv_label_set_text_fmt(lapOneText, "#%d   %d:%d:%d", (lapNr - 1), lapBuffer[1]->mins, lapBuffer[1]->secs, lapBuffer[1]->msecs);
        }
        if (lapBuffer[0]) {
          lv_label_set_text_fmt(lapTwoText, "#%d   %d:%d:%d", lapNr, lapBuffer[0]->mins, lapBuffer[0]->secs, lapBuffer[0]->msecs);
        }
        // Reset the bool to avoid setting the text in each cycle
        lapPressed = false;
      }

      if (currentEvent == Events::PAUSE) {
        // Reset the start time
        startTime = 0;
        // Store the current time elapsed in cache
        oldTimeElapsed += timeElapsed;
        currentState = States::HALTED;
      }
      break;
    }
    case States::HALTED: {
      lv_label_set_text(txtPlayPause, Symbols::play);
      lv_label_set_text(txtStopLap, Symbols::stop);

      if (currentEvent == Events::PLAY) {
        startTime = xTaskGetTickCount();
        currentState = States::RUNNING;
      }
      if (currentEvent == Events::STOP) {
        currentState = States::INIT;
        oldTimeElapsed = 0;
      }
      break;
    }
  }
  return running;
}

bool StopWatch::OnButtonPushed() {
  running = false;
  return true;
}

void StopWatch::playPauseBtnEventHandler(lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (currentState == States::INIT) {
      currentEvent = Events::PLAY;
    } else {
      // Simple Toggle for play/pause
      currentEvent = (currentEvent == Events::PLAY ? Events::PAUSE : Events::PLAY);
    }
  }
}

void StopWatch::stopLapBtnEventHandler(lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    // If running, then this button is used to save laps
    if (currentState == States::RUNNING) {
      lapBuffer.addLaps(currentTimeSeparated);
      lapNr++;
      lapPressed = true;

    } else if (currentState == States::HALTED) {
      currentEvent = Events::STOP;
    } else {
      // Not possible to reach here. Do nothing.
    }
  }
}
