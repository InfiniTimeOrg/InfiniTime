#include "displayapp/screens/StopWatch.h"

#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;
using namespace Pinetime::Controllers;

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
    stopWatch->PlayPauseBtnEventHandler(event);
  }

  void stop_lap_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    stopWatch->StopLapBtnEventHandler(event);
  }
}

StopWatch::StopWatch(DisplayApp* app, System::SystemTask& systemTask,
                     StopWatchController& stopWatchController)
  : Screen(app), systemTask {systemTask}, stopWatchController {stopWatchController} {

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_label_set_text_static(time, "00:00");
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_CENTER, 0, -45);

  msecTime = lv_label_create(lv_scr_act(), nullptr);
  // lv_obj_set_style_local_text_font(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
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
  txtStopLap = lv_label_create(btnStopLap, nullptr);
  lv_label_set_text_static(txtStopLap, Symbols::stop);

  lapText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lapText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_align(lapText, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 50, 30);
  lv_label_set_text_static(lapText, "");

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  // Figure out what the current state of the stopwatch is and select the correct display
  if (stopWatchController.IsCleared()) {
    DisplayReset();
  } else  {
    if (stopWatchController.GetLapCount() > 0) {
      UpdateLaps();
    }
    UpdateTime();

    if (stopWatchController.IsRunning()) {
      lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
      lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);
      DisplayStart();
    } else if (stopWatchController.IsPaused()) {
      lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
      lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT);
      DisplayPause();
    }
  }
}

StopWatch::~StopWatch() {
  lv_task_del(taskRefresh);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void StopWatch::DisplayReset() {
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);

  lv_label_set_text_static(time, "00:00");
  lv_label_set_text_static(msecTime, "00");

  lv_label_set_text_static(lapText, "");
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);
}

void StopWatch::DisplayStart() {
  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
  lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
  lv_label_set_text_static(txtPlayPause, Symbols::pause);
  lv_label_set_text_static(txtStopLap, Symbols::lapsFlag);
  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

void StopWatch::DisplayPause() {
  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
  lv_label_set_text_static(txtPlayPause, Symbols::play);
  lv_label_set_text_static(txtStopLap, Symbols::stop);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void StopWatch::UpdateTime() {
  TimeSeparated_t currentTimeSeparated = convertTicksToTimeSegments(stopWatchController.GetElapsedTime());

  lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
  lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.hundredths);
}

void StopWatch::UpdateLaps() {
  lv_label_set_text(lapText, "");

  for (int i = 0; i < displayedLaps; i++) {
    LapInfo* lap = stopWatchController.LastLap(i);

    if (lap->count != 0) {
      TimeSeparated_t laptime = convertTicksToTimeSegments(lap->time);
      char buffer[16];
      sprintf(buffer, "#%2d   %2d:%02d.%02d\n", lap->count, laptime.mins, laptime.secs, laptime.hundredths);
      lv_label_ins_text(lapText, LV_LABEL_POS_LAST, buffer);
    }
  }
}

void StopWatch::Refresh() {
  if (stopWatchController.IsRunning()) {
    UpdateTime();
  }
}

void StopWatch::PlayPauseBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  if (stopWatchController.IsCleared() || stopWatchController.IsPaused()) {
    stopWatchController.Start();
    DisplayStart();
  } else if (stopWatchController.IsRunning()) {
    stopWatchController.Pause();
    DisplayPause();
  }
}

void StopWatch::StopLapBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  // If running, then this button is used to save laps
  if (stopWatchController.IsRunning()) {
    stopWatchController.PushLap();

    UpdateLaps();
  } else if (stopWatchController.IsPaused()) {
    stopWatchController.Clear();
    DisplayReset();
  }
}

bool StopWatch::OnButtonPushed() {
  if (stopWatchController.IsRunning()) {
    stopWatchController.Pause();
    DisplayPause();
    return true;
  }
  return false;
}
