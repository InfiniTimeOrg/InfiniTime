#include "StopWatch.h"

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <lvgl/lvgl.h>
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

StopWatch::StopWatch(DisplayApp* app,
                     System::SystemTask& systemTask,
                     Controllers::DateTime& dateTimeController,
                     Controllers::StopWatch& stopWatchController)
  : Screen(app),
    systemTask {systemTask},
    dateTimeController {dateTimeController},
    stopWatchController {stopWatchController},
    timeElapsed {},
    currentTimeSeparated {} {

  // Running time
  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text(time, "00:00");
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_CENTER, 0, -45);

  // Create millisecond label
  msecTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text(msecTime, "00");
  lv_obj_align(msecTime, lv_scr_act(), LV_ALIGN_CENTER, 0, 3);

  // Create play/pause button
  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, play_pause_event_handler);
  lv_obj_set_height(btnPlayPause, 50);
  lv_obj_set_width(btnPlayPause, 115);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  lv_label_set_text(txtPlayPause, Symbols::play);

  // Create stop/lap button
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

  // Create first lap text label
  lapOneText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lapOneText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_align(lapOneText, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 50, 30);
  lv_label_set_text(lapOneText, "");

  // Create second lap text label
  lapTwoText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lapTwoText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_align(lapTwoText, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 50, 55);
  lv_label_set_text(lapTwoText, "");

  // Create Date time label
  dateTime = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(dateTime, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  lv_label_set_align(dateTime, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(dateTime, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_set_style_local_text_color(dateTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  // Figure out what the current state of the stopwatch is and select the correct display
  if (stopWatchController.isCleared()) {
    displayCleared();
  } else  {
    // Add laps if there are any
    if (stopWatchController.getLapCount() > 0) {
      updateLaps();
    }

    if (stopWatchController.isRunning()) {
      displayRunning();
    } else if (stopWatchController.isPaused()) {
      displayPaused();
      currentTimeSeparated = convertTicksToTimeSegments(stopWatchController.getElapsedPreviously());

      lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
      lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.hundredths);
      lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
      lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT);
    }
  }
}

StopWatch::~StopWatch() {
  lv_task_del(taskRefresh);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void StopWatch::displayCleared() {
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  lv_label_set_text(time, "00:00");
  lv_label_set_text(msecTime, "00");

  lv_label_set_text(lapOneText, "");
  lv_label_set_text(lapTwoText, "");

  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);
}

void StopWatch::displayRunning() {
  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
  lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_label_set_text(txtPlayPause, Symbols::pause);
  lv_label_set_text(txtStopLap, Symbols::lapsFlag);

  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

void StopWatch::displayPaused() {
  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);

  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);

  lv_label_set_text(txtPlayPause, Symbols::play);
  lv_label_set_text(txtStopLap, Symbols::stop);

  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void StopWatch::updateLaps() {
  Pinetime::Controllers::LapInfo_t* lap1 = stopWatchController.lastLap();
  Pinetime::Controllers::LapInfo_t* lap2 = stopWatchController.lastLap(1);

  if (lap1->count != 0) {
    TimeSeparated_t laptime = convertTicksToTimeSegments(lap1->time);
    lv_label_set_text_fmt(lapOneText, "#%2d   %2d:%02d.%02d", lap1->count, laptime.mins, laptime.secs, laptime.hundredths);
  }
  if (lap2->count != 0) {
    TimeSeparated_t laptime = convertTicksToTimeSegments(lap2->time);
    lv_label_set_text_fmt(lapTwoText, "#%2d   %2d:%02d.%02d", lap2->count, laptime.mins, laptime.secs, laptime.hundredths);
  }
}

void StopWatch::Refresh() {
  lv_label_set_text_fmt(dateTime, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  if (stopWatchController.isRunning()) {
    timeElapsed = calculateDelta(stopWatchController.getStart(), xTaskGetTickCount());
    currentTimeSeparated = convertTicksToTimeSegments((stopWatchController.getElapsedPreviously() + timeElapsed));

    lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
    lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.hundredths);
  }
}

void StopWatch::playPauseBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  if (stopWatchController.isCleared() || stopWatchController.isPaused()) {
    stopWatchController.start(xTaskGetTickCount());
    displayRunning();
  } else if (stopWatchController.isRunning()) {
    stopWatchController.pause(xTaskGetTickCount());
    displayPaused();
  }
}

void StopWatch::stopLapBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  // If running, then this button is used to save laps
  if (stopWatchController.isRunning()) {
    TickType_t currentTime =
      stopWatchController.getElapsedPreviously() + calculateDelta(stopWatchController.getStart(), xTaskGetTickCount());
    stopWatchController.pushLap(currentTime);

    updateLaps();
  } else if (stopWatchController.isPaused()) {
    stopWatchController.clear();
    displayCleared();
  }
}
