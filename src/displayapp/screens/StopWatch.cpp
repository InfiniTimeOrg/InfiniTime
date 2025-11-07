#include "displayapp/screens/StopWatch.h"

#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;
using namespace Pinetime::Controllers;

namespace {
  TimeSeparated ConvertTicksToTimeSegments(const TickType_t timeElapsed) {
    const uint32_t timeElapsedSecs = timeElapsed / configTICK_RATE_HZ;
    const uint16_t timeElapsedFraction = timeElapsed % configTICK_RATE_HZ;

    const uint8_t hundredths = timeElapsedFraction * 100 / configTICK_RATE_HZ;
    const uint8_t secs = (timeElapsedSecs) % 60;
    const uint8_t mins = (timeElapsedSecs / 60) % 60;
    const uint16_t hours = (timeElapsedSecs / 60) / 60;
    return TimeSeparated {hours, mins, secs, hundredths, timeElapsedSecs};
  }

  void PlayPauseEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      stopWatch->PlayPauseBtnEventHandler();
    }
  }

  void StopLapEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      stopWatch->StopLapBtnEventHandler();
    }
  }

  constexpr TickType_t blinkInterval = pdMS_TO_TICKS(1000);
}

StopWatch::StopWatch(System::SystemTask& systemTask, StopWatchController& stopWatchController)
  : wakeLock(systemTask), stopWatchController {stopWatchController} {
  static constexpr uint8_t btnWidth = 115;
  static constexpr uint8_t btnHeight = 80;
  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, PlayPauseEventHandler);
  lv_obj_set_size(btnPlayPause, btnWidth, btnHeight);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);

  btnStopLap = lv_btn_create(lv_scr_act(), nullptr);
  btnStopLap->user_data = this;
  lv_obj_set_event_cb(btnStopLap, StopLapEventHandler);
  lv_obj_set_size(btnStopLap, btnWidth, btnHeight);
  lv_obj_align(btnStopLap, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  txtStopLap = lv_label_create(btnStopLap, nullptr);
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);

  lapText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lapText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(lapText, "");
  lv_label_set_long_mode(lapText, LV_LABEL_LONG_BREAK);
  lv_label_set_align(lapText, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_width(lapText, LV_HOR_RES_MAX);
  lv_obj_align(lapText, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -btnHeight - 2);

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DISABLED, Colors::lightGray);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_label_set_text_static(time, "00:00");
  lv_label_set_long_mode(time, LV_LABEL_LONG_CROP);
  lv_label_set_align(time, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_width(time, LV_HOR_RES_MAX);
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

  msecTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DISABLED, Colors::lightGray);
  lv_label_set_text_static(msecTime, "00");
  lv_obj_align(msecTime, time, LV_ALIGN_OUT_BOTTOM_MID, 0, -2);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  // Figure out what the current state of the stopwatch is and select the correct display
  if (stopWatchController.IsCleared()) {
    DisplayCleared();
  } else {
    if (stopWatchController.GetMaxLapNumber() > 0) {
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

  SetHoursVisible(false);

  lv_label_set_text_static(lapText, "");
  lv_label_set_text_static(txtPlayPause, Symbols::play);
  lv_label_set_text_static(txtStopLap, Symbols::lapsFlag);
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);
}

void StopWatch::RenderTime() {
  TimeSeparated elapsedTime = ConvertTicksToTimeSegments(stopWatchController.GetElapsedTime());
  renderedSeconds = elapsedTime.epochSecs;
  if (renderedSeconds.IsUpdated()) {
    SetHoursVisible(elapsedTime.hours != 0);
    if (!hoursVisible) {
      lv_label_set_text_fmt(time, "%02d:%02d", elapsedTime.mins, elapsedTime.secs);
    } else {
      lv_label_set_text_fmt(time, "%02d:%02d:%02d", elapsedTime.hours, elapsedTime.mins, elapsedTime.secs);
    }
  }
  lv_label_set_text_fmt(msecTime, "%02d", elapsedTime.hundredths);
}

void StopWatch::RenderPause() {
  const TickType_t currentTime = xTaskGetTickCount();
  if (currentTime - lastBlinkTime > blinkInterval) {
    lastBlinkTime = currentTime;
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
  for (int i = displayedLaps - 1; i >= 0; i--) {
    std::optional<LapInfo> lap = stopWatchController.GetLapFromHistory(i);

    if (lap) {
      TimeSeparated laptime = ConvertTicksToTimeSegments(lap->timeSinceStart);
      char buffer[19];
      if (laptime.hours > 0) {
        snprintf(buffer,
                 sizeof(buffer),
                 "\n#%-3d %3d:%02d:%02d.%02d",
                 lap->number,
                 laptime.hours,
                 laptime.mins,
                 laptime.secs,
                 laptime.hundredths);
      } else if (laptime.mins > 0) {
        snprintf(buffer, sizeof(buffer), "\n#%-3d     %2d:%02d.%02d", lap->number, laptime.mins, laptime.secs, laptime.hundredths);
      } else {
        snprintf(buffer, sizeof(buffer), "\n#%-3d        %2d.%02d", lap->number, laptime.secs, laptime.hundredths);
      }
      lv_label_ins_text(lapText, LV_LABEL_POS_LAST, buffer);
    }
  }
  lv_obj_realign(lapText);
}

void StopWatch::SetHoursVisible(bool visible) {
  if (hoursVisible != visible) {
    lv_font_t* font = visible ? &jetbrains_mono_42 : &jetbrains_mono_76;
    lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font);
    lv_obj_set_height(time, font->line_height);
    lv_obj_align(time, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, visible ? 5 : 0);
    lv_obj_align(msecTime, time, LV_ALIGN_OUT_BOTTOM_MID, 0, visible ? 5 : -2);
    displayedLaps = visible ? 4 : 3;
    hoursVisible = visible;
    if (stopWatchController.GetLapFromHistory(0)) {
      RenderLaps();
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
    OnPause();
  }
}

void StopWatch::StopLapBtnEventHandler() {
  if (stopWatchController.IsRunning()) {
    stopWatchController.AddLapToHistory();
    RenderLaps();
  } else if (stopWatchController.IsPaused()) {
    stopWatchController.Clear();
    DisplayCleared();
    wakeLock.Release();
  }
}

bool StopWatch::OnButtonPushed() {
  if (stopWatchController.IsRunning()) {
    OnPause();
    return true;
  }
  return false;
}

void StopWatch::OnPause() {
  stopWatchController.Pause();
  lastBlinkTime = xTaskGetTickCount();
  RenderTime(); // make sure displayed time is not stale
  DisplayPaused();
  wakeLock.Release();
}
