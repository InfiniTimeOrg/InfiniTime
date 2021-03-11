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
  std::tuple<int, int, int> convertTicksToTimeSegments(const TickType_t timeElapsed) {
    const int timeElapsedMillis = (static_cast<float>(timeElapsed) / static_cast<float>(configTICK_RATE_HZ)) * 1000;

    const int milliSecs = (timeElapsedMillis % 1000) / 10; // Get only the first two digits and ignore the last
    const int secs = (timeElapsedMillis / 1000) % 60;
    const int mins = (timeElapsedMillis / 1000) / 60;
    return std::make_tuple(mins, secs, milliSecs);
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

StopWatch::StopWatch(DisplayApp* app, const Pinetime::Controllers::DateTime& dateTime)
  : Screen(app), dateTime {dateTime}, running {true}, currentState {States::INIT}, currentEvent {Events::STOP}, startTime {},
    oldTimeElapsed {} {

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -50);

  msecTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_align(msecTime, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 113, 0);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, play_pause_event_handler);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  lv_label_set_text(txtPlayPause, Symbols::play);
}

StopWatch::~StopWatch() {
  lv_obj_clean(lv_scr_act());
}

bool StopWatch::Refresh() {

  switch (currentState) {
    // Init state when an user first opens the app
    // and when a stop/reset button is pressed
    case States::INIT: {
      // The initial default value
      lv_label_set_text(time, "00:00");
      lv_label_set_text(msecTime, "00");
      if (currentEvent == Events::PLAY) {
        startTime = xTaskGetTickCount();
        currentState = States::RUNNING;
      }
      break;
    }
    case States::RUNNING: {
      lv_label_set_text(txtPlayPause, Symbols::pause);

      const auto timeElapsed = calculateDelta(startTime, xTaskGetTickCount());
      const auto timeSeparated = convertTicksToTimeSegments((oldTimeElapsed + timeElapsed));

      lv_label_set_text_fmt(time, "%02d:%02d", std::get<0>(timeSeparated), std::get<1>(timeSeparated));
      lv_label_set_text_fmt(msecTime, "%02d", std::get<2>(timeSeparated));

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

      if (currentEvent == Events::PLAY) {
        startTime = xTaskGetTickCount();
        currentState = States::RUNNING;
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

bool StopWatch::OnTouchEvent(uint16_t x, uint16_t y) {
  if (y < 60) {
    playPauseBtnEventHandler(LV_EVENT_CLICKED);
  }
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