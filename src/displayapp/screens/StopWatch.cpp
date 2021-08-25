#include "displayapp/screens/StopWatch.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  TimeSeparated_t convertTicksToTimeSegments(const TickType_t timeElapsed) {
    const int timeElapsedMillis = (static_cast<float>(timeElapsed) / static_cast<float>(configTICK_RATE_HZ)) * 1000;

    const int hundredths = (timeElapsedMillis % 1000) / 10; // Get only the first two digits and ignore the last
    const int secs = (timeElapsedMillis / 1000) % 60;
    const int mins = (timeElapsedMillis / 1000) / 60;
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
  // lv_obj_set_style_local_text_font(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text(msecTime, "00");
  lv_obj_align(msecTime, lv_scr_act(), LV_ALIGN_CENTER, 0, 3);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, play_pause_event_handler);
  lv_obj_set_size(btnPlayPause, 115, 50);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  lv_label_set_text(txtPlayPause, Symbols::play);

  btnStopLap = lv_btn_create(lv_scr_act(), nullptr);
  btnStopLap->user_data = this;
  lv_obj_set_event_cb(btnStopLap, stop_lap_event_handler);
  lv_obj_set_size(btnStopLap, 115, 50);
  lv_obj_align(btnStopLap, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x080808));
  txtStopLap = lv_label_create(btnStopLap, nullptr);
  lv_obj_set_style_local_text_color(txtStopLap, LV_BTN_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x888888));
  lv_label_set_text(txtStopLap, Symbols::stop);
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);

  lapText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lapText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
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
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  lv_label_set_text(lapText, "");
  savedLapsCount = 0;
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);

  SetTime(0);
}

void StopWatch::Start() {
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

void StopWatch::Pause() {
  startTime = 0;
  // Store the current time elapsed in cache
  oldTimeElapsed = lapTimes[savedLapsCount];
  currentState = States::Halted;
  lv_label_set_text(txtPlayPause, Symbols::play);
  lv_label_set_text(txtStopLap, Symbols::stop);
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
            "#%d %d:%02d.%02d %d:%02d.%02d\n",
            i + 1,
            lapTime.mins,
            lapTime.secs,
            lapTime.hundredths,
            runTime.mins,
            runTime.secs,
            runTime.hundredths);

    allLapsText.append(buffer);
  }
  lv_label_set_text(lapText, allLapsText.c_str());
}

void StopWatch::SetTime(TickType_t timeToSet) {
  TimeSeparated_t currentTimeSeparated = convertTicksToTimeSegments(timeToSet);

  lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
  lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.hundredths);

  if (savedLapsCount > 0) {
    lv_obj_align(time, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
  } else {
    lv_obj_align(time, lv_scr_act(), LV_ALIGN_CENTER, 0, -45);
  }
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
    DisplayLaps(std::max(savedLapsCount - 5, 0));
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
