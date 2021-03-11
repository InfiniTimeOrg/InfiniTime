#include "StopWatch.h"

#include "Screen.h"
#include "lvgl/lvgl.h"
#include "projdefs.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include <tuple>

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
using namespace Pinetime::Applications::Screens;

StopWatch::StopWatch(DisplayApp* app, const Pinetime::Controllers::DateTime& dateTime)
  : Screen(app), dateTime {dateTime}, running {true}, currentState {States::INIT}, currentEvent {Events::PLAY}, startTime {},
    timeElapsed {} {

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 0);

  msecTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_align(msecTime, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 115, 60);
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
      auto timeElapsed = calculateDelta(startTime, xTaskGetTickCount());
      auto timeSeparated = convertTicksToTimeSegments(timeElapsed);
      lv_label_set_text_fmt(time, "%02d:%02d", std::get<0>(timeSeparated), std::get<1>(timeSeparated));
      lv_label_set_text_fmt(msecTime, "%02d", std::get<2>(timeSeparated));
      break;
    }
    case States::HALTED: {

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
  return true;
}