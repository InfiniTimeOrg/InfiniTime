#include "displayapp/screens/StopWatch.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  TimeSeparated_t convertTicksToTimeSegments(const TickType_t timeElapsed) {
    TickType_t timeElapsedCentis = timeElapsed * 100 / configTICK_RATE_HZ;

    const uint8_t hundredths = (timeElapsedCentis % 100);
    const uint8_t secs = (timeElapsedCentis / 100) % 60;
    const uint32_t mins = (timeElapsedCentis / 100) / 60;
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

StopWatch::StopWatch(DisplayApp* app, System::SystemTask& systemTask) : Screen(app), systemTask {systemTask} {

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text(time, "00:00");
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_CENTER, 0, -45);

  msecTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text(msecTime, "00");
  lv_obj_align(msecTime, lv_scr_act(), LV_ALIGN_CENTER, 0, 3);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, play_pause_event_handler);
  lv_obj_set_size(btnPlayPause, 115, 50);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lv_obj_set_style_local_value_str(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::play);

  btnStopLap = lv_btn_create(lv_scr_act(), nullptr);
  btnStopLap->user_data = this;
  lv_obj_set_event_cb(btnStopLap, stop_lap_event_handler);
  lv_obj_set_size(btnStopLap, 115, 50);
  lv_obj_align(btnStopLap, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x080808));
  lv_obj_set_style_local_text_color(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x888888));
  lv_obj_set_style_local_value_str(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::stop);
  lv_obj_set_style_local_value_str(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DISABLED, Symbols::stop);
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);

  lapText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lapText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_SILVER);
  lv_obj_align(lapText, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 72);
  lv_label_set_text(lapText, "");

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
  lv_obj_set_hidden(msecTime, false);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  lv_label_set_text(lapText, "");
  savedLapsCount = 0;
  displayedLapsStart = 0;
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);

  lv_obj_align(time, lv_scr_act(), LV_ALIGN_CENTER, 0, -45);
  SetTime(0);
}

void StopWatch::Start() {
  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
  lv_obj_set_style_local_value_str(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::pause);
  lv_obj_set_style_local_value_str(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::lapsFlag);

  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);

  startTime = xTaskGetTickCount();
  currentState = States::Running;
  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);

  displayedLapsStart = std::max(savedLapsCount - 5, 0);
  DisplayLaps(displayedLapsStart);
}

void StopWatch::Pause() {
  startTime = 0;
  // Store the current time elapsed in cache
  oldTimeElapsed = lapTimes[savedLapsCount];
  currentState = States::Halted;

  lv_obj_set_style_local_value_str(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::play);
  lv_obj_set_style_local_value_str(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::stop);

  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);

  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void StopWatch::DisplayLaps(uint8_t startLap) {
  char buffer[22];
  std::string allLapsText = "";
  TimeSeparated_t lapTime;
  TimeSeparated_t runTime;
  for (uint8_t i = startLap; i < savedLapsCount && i < startLap + 5 && i < maxLapCount; i++) {
    runTime = convertTicksToTimeSegments(lapTimes[i]);
    if (i > 0) {
      lapTime = convertTicksToTimeSegments(lapTimes[i] - lapTimes[i - 1]);
    } else {
      lapTime = runTime;
    }
    sprintf(buffer,
            "#%-2d %2d:%02d.%1d %2d:%02d.%1d\n",
            i + 1,
            std::min(lapTime.mins, 99),
            lapTime.secs,
            lapTime.hundredths / 10,
            std::min(runTime.mins, 99),
            runTime.secs,
            runTime.hundredths / 10);

    allLapsText.append(buffer);
  }
  lv_label_set_text(lapText, allLapsText.c_str());
}

void StopWatch::SetTime(TickType_t timeToSet) {
  TimeSeparated_t currentTimeSeparated = convertTicksToTimeSegments(timeToSet);

  lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
  lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.hundredths);
}

void StopWatch::Refresh() {
  if (currentState == States::Running) {
    lapTimes[savedLapsCount] = oldTimeElapsed + xTaskGetTickCount() - startTime;
    SetTime(lapTimes[savedLapsCount]);
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
    if (savedLapsCount < maxLapCount) {
      savedLapsCount++;
    }
    lv_obj_align(time, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
    lv_obj_set_hidden(msecTime, true);
    displayedLapsStart = std::max(savedLapsCount - 5, 0);
    DisplayLaps(displayedLapsStart);
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

bool StopWatch::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (currentState != States::Halted) {
    return false;
  }

  switch (event) {
    case TouchEvents::SwipeUp:
      displayedLapsStart = std::min({displayedLapsStart + 5, maxLapCount - 5, savedLapsCount - 5});
      break;
    case TouchEvents::SwipeDown:
      displayedLapsStart = std::max(displayedLapsStart - 5, 0);
      break;
    default:
      return false;
  }
  DisplayLaps(displayedLapsStart);

  return true;
}
