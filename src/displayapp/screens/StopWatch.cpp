#include "StopWatch.h"

#include "Screen.h"
#include "lvgl/lvgl.h"

#include <tuple>

// Anonymous namespace for local functions
namespace {
  std::tuple<int, int, int> convertMilliSecsToSegments(const int64_t& currentTime) {
    const int milliSecs = (currentTime % 1000); // Get only the first two digits and ignore the last
    const int secs = (currentTime / 1000) % 60;
    const int mins = (currentTime / 1000) / 60;
    return std::make_tuple(mins, secs, milliSecs);
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
        startTime = dateTime.CurrentDateTime();
        currentState = States::RUNNING;
      }
      break;
    }
    case States::RUNNING: {
      auto delta = std::chrono::duration_cast<std::chrono::microseconds>(dateTime.CurrentDateTime() - startTime);
      timeElapsed = delta.count();
      auto timeSeparated = convertMilliSecsToSegments(timeElapsed);
      lv_label_set_text_fmt(time, "%02d:%02d", std::get<0>(timeSeparated), std::get<1>(timeSeparated));
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