#include "displayapp/screens/StopWatch.h"

#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;
using namespace Pinetime::Controllers;

namespace {
  TimeSeparated convertTicksToTimeSegments(const TickType_t timeElapsed) {
    // Centiseconds
    const int timeElapsedCentis = timeElapsed * 100 / configTICK_RATE_HZ;

    const int hundredths = (timeElapsedCentis % 100);
    const int secs = (timeElapsedCentis / 100) % 60;
    const int mins = ((timeElapsedCentis / 100) / 60) % 60;
    const int hours = ((timeElapsedCentis / 100) / 60) / 60;
    return TimeSeparated {hours, mins, secs, hundredths};
  }

  void play_pause_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      stopWatch->PlayPauseBtnEventHandler();
    }
  }

  void stop_lap_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      stopWatch->StopLapBtnEventHandler();
    }
  }

  constexpr TickType_t blinkInterval = pdMS_TO_TICKS(1000);
}

StopWatch::StopWatch(System::SystemTask& systemTask,
                     StopWatchController& stopWatchController)
  : wakeLock(systemTask), stopWatchController {stopWatchController} {
  static constexpr uint8_t btnWidth = 115;
  static constexpr uint8_t btnHeight = 80;
  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, play_pause_event_handler);
  lv_obj_set_size(btnPlayPause, btnWidth, btnHeight);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);

  btnStopLap = lv_btn_create(lv_scr_act(), nullptr);
  btnStopLap->user_data = this;
  lv_obj_set_event_cb(btnStopLap, stop_lap_event_handler);
  lv_obj_set_size(btnStopLap, btnWidth, btnHeight);
  lv_obj_align(btnStopLap, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  txtStopLap = lv_label_create(btnStopLap, nullptr);
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);

  lapText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lapText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(lapText, "\n");
  lv_label_set_long_mode(lapText, LV_LABEL_LONG_BREAK);
  lv_label_set_align(lapText, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_width(lapText, LV_HOR_RES_MAX);
  lv_obj_align(lapText, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -btnHeight);

  msecTime = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(msecTime, "00");
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DISABLED, Colors::lightGray);
  lv_obj_align(msecTime, lapText, LV_ALIGN_OUT_TOP_MID, 0, 0);

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_label_set_text_static(time, "00:00");
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DISABLED, Colors::lightGray);
  lv_obj_align(time, msecTime, LV_ALIGN_OUT_TOP_MID, 0, 0);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  // Figure out what the current state of the stopwatch is and select the correct display
  if (stopWatchController.IsCleared()) {
    DisplayCleared();
  } else  {
    if (stopWatchController.GetLapCount() > 0) {
      RenderLaps();
    }
    RenderTime();

    if (stopWatchController.IsRunning()) {
      lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
      lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);
      DisplayStarted();
      wakeLock.Lock();
    } else if (stopWatchController.IsPaused()) {
      lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
      lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT);
      DisplayPaused();
    }
  }
}

StopWatch::~StopWatch() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void StopWatch::DisplayPaused() {
  lv_obj_set_style_local_bg_color(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::blue);
  lv_label_set_text_static(txtPlayPause, Symbols::play);
  lv_label_set_text_static(txtStopLap, Symbols::stop);
}

void StopWatch::DisplayStarted() {
  lv_obj_set_state(time, LV_STATE_DEFAULT);
  lv_obj_set_state(msecTime, LV_STATE_DEFAULT);
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_style_local_bg_color(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);

  lv_label_set_text_static(txtPlayPause, Symbols::pause);
  lv_label_set_text_static(txtStopLap, Symbols::lapsFlag);

  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
  lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT);
}

void StopWatch::DisplayCleared() {
  lv_obj_set_state(time, LV_STATE_DISABLED);
  lv_obj_set_state(msecTime, LV_STATE_DISABLED);
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::blue);

  lv_label_set_text_static(time, "00:00");
  lv_label_set_text_static(msecTime, "00");

  if (isHoursLabelUpdated) {
    lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
    lv_obj_realign(time);
    isHoursLabelUpdated = false;
  }

  lv_label_set_text_static(lapText, "");
  lv_label_set_text_static(txtPlayPause, Symbols::play);
  lv_label_set_text_static(txtStopLap, Symbols::lapsFlag);
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);
}

void StopWatch::RenderTime() {
  TimeSeparated currentTimeSeparated = convertTicksToTimeSegments(stopWatchController.GetElapsedTime());
  if (currentTimeSeparated.hours == 0) {
    lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
  } else {
    lv_label_set_text_fmt(time, "%02d:%02d:%02d", currentTimeSeparated.hours, currentTimeSeparated.mins, currentTimeSeparated.secs);
    if (!isHoursLabelUpdated) {
      lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
      lv_obj_realign(time);
      isHoursLabelUpdated = true;
    }
  }
  lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.hundredths);
}

void StopWatch::RenderPause() {
  const TickType_t currentTime = xTaskGetTickCount();
  if (currentTime > blinkTime) {
    blinkTime = currentTime + blinkInterval;
    if (lv_obj_get_state(time, LV_LABEL_PART_MAIN) == LV_STATE_DEFAULT) {
      lv_obj_set_state(time, LV_STATE_DISABLED);
      lv_obj_set_state(msecTime, LV_STATE_DISABLED);
    } else {
      lv_obj_set_state(time, LV_STATE_DEFAULT);
      lv_obj_set_state(msecTime, LV_STATE_DEFAULT);
    }
  }
}

void StopWatch::RenderLaps() {
  lv_label_set_text(lapText, "");
  for (int i = 0; i < displayedLaps; i++) {
    LapInfo* lap = stopWatchController.LastLap(i);

    if (lap->count != 0) {
      TimeSeparated laptime = convertTicksToTimeSegments(lap->time);
      char buffer[16];
      sprintf(buffer, "#%2d   %2d:%02d.%02d\n", lap->count, laptime.mins, laptime.secs, laptime.hundredths);
      lv_label_ins_text(lapText, LV_LABEL_POS_LAST, buffer);
    }
  }
}

void StopWatch::Refresh() {
  if (stopWatchController.IsRunning()) {
    RenderTime();
  } else if (stopWatchController.IsPaused()) {
    RenderPause();
  }
}

void StopWatch::PlayPauseBtnEventHandler() {
  if (stopWatchController.IsCleared() || stopWatchController.IsPaused()) {
    stopWatchController.Start();
    DisplayStarted();
    wakeLock.Lock();
  } else if (stopWatchController.IsRunning()) {
    stopWatchController.Pause();
    blinkTime = xTaskGetTickCount() + blinkInterval;
    DisplayPaused();
    wakeLock.Release();
  }
}

void StopWatch::StopLapBtnEventHandler() {
  if (stopWatchController.IsRunning()) {
    stopWatchController.PushLap();
    RenderLaps();
  } else if (stopWatchController.IsPaused()) {
    stopWatchController.Clear();
    DisplayCleared();
    wakeLock.Release();
  }
}

bool StopWatch::OnButtonPushed() {
  if (stopWatchController.IsRunning()) {
    stopWatchController.Pause();
    DisplayPaused();
    wakeLock.Release();
    return true;
  }
  return false;
}
