#include "displayapp/screens/TrackTime.h"

#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  TimePretty_t convertTicksToTimeSegments(const TickType_t timeElapsed) {
    // Centiseconds
    const int timeElapsedCentis = timeElapsed * 100 / configTICK_RATE_HZ;

    const int hundredths = (timeElapsedCentis % 100);
    const int secs = (timeElapsedCentis / 100) % 60;
    const int mins = (timeElapsedCentis / 100) / 60;
    return TimePretty_t {mins, secs, hundredths};
  }

  void play_pause_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<TrackTime*>(obj->user_data);
    stopWatch->playPauseBtnEventHandler(event);
  }

  void stop_lap_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<TrackTime*>(obj->user_data);
    stopWatch->stopLapBtnEventHandler(event);
  }

  void btnHandler(lv_obj_t* obj, lv_event_t event) {
    auto* tracktime = static_cast<TrackTime*>(obj->user_data);
    tracktime->handleModeUpdate(event);
  }
}

TrackTime::TrackTime(DisplayApp* app, System::SystemTask& systemTask) : Screen(app), systemTask {systemTask} {

  btnm1 = lv_btnmatrix_create(lv_scr_act(), nullptr);
  btnm1->user_data = this;
  lv_obj_align(btnm1, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_set_size(btnm1, 240, 180);
  lv_btnmatrix_set_map(btnm1, btnm_map);
  lv_btnmatrix_set_btn_ctrl_all(btnm1, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_obj_set_event_cb(btnm1, btnHandler);
  lv_btnmatrix_set_one_check(btnm1, true);

  title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Total: ");
  lv_label_set_align(title, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, -20);
}

TrackTime::~TrackTime() {
  lv_task_del(taskRefresh);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void TrackTime::Reset() {
  currentState = TrackerStates::Init;
  oldTimeElapsed = 0;
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);

  lv_label_set_text_static(time, "00:00");
  lv_label_set_text_static(msecTime, "00");

  lv_label_set_text_static(lapText, "");
  lapsDone = 0;
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);
}

void TrackTime::Start() {
  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
  lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
  lv_label_set_text_static(txtPlayPause, Symbols::pause);
  lv_label_set_text_static(txtStopLap, Symbols::lapsFlag);
  startTime = xTaskGetTickCount();
  currentState = TrackerStates::Running;
  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

void TrackTime::Pause() {
  startTime = 0;
  // Store the current time elapsed in cache
  oldTimeElapsed = laps[lapsDone];
  currentState = TrackerStates::Halted;
  lv_label_set_text_static(txtPlayPause, Symbols::play);
  lv_label_set_text_static(txtStopLap, Symbols::stop);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void TrackTime::Refresh() {
  if (currentState == TrackerStates::Running) {
    laps[lapsDone] = oldTimeElapsed + xTaskGetTickCount() - startTime;

    TimePretty_t currentTimeSeparated = convertTicksToTimeSegments(laps[lapsDone]);
    lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
    lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.hundredths);
  }
}

void TrackTime::playPauseBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  if (currentState == TrackerStates::Init) {
    Start();
  } else if (currentState == TrackerStates::Running) {
    Pause();
  } else if (currentState == TrackerStates::Halted) {
    Start();
  }
}

void TrackTime::stopLapBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  // If running, then this button is used to save laps
  if (currentState == TrackerStates::Running) {
    lv_label_set_text(lapText, "");
    lapsDone = std::min(lapsDone + 1, maxLapCount);
    for (int i = lapsDone - displayedLaps; i < lapsDone; i++) {
      if (i < 0) {
        lv_label_ins_text(lapText, LV_LABEL_POS_LAST, "\n");
        continue;
      }
      TimePretty_t times = convertTicksToTimeSegments(laps[i]);
      char buffer[16];
      sprintf(buffer, "#%2d   %2d:%02d.%02d\n", i + 1, times.mins, times.secs, times.hundredths);
      lv_label_ins_text(lapText, LV_LABEL_POS_LAST, buffer);
    }
  } else if (currentState == TrackerStates::Halted) {
    Reset();
  }
}

void TrackTime::handleModeUpdate(lv_event_t event) {
  if (event != LV_EVENT_VALUE_CHANGED) {
    return;
  }
  const char* txt = lv_btnmatrix_get_active_btn_text(btnm1);
  uint16_t btnId = lv_btnmatrix_get_active_btn(btnm1);
  printf("%s %i was pressed\n", txt, btnId);
  oldBtnPressed = oldBtnPressed + 1;
  printf("demo %i\n", oldBtnPressed);
}

bool TrackTime::OnButtonPushed() {
  oldBtnPressed = oldBtnPressed + 1;
  printf("sanity %i\n", oldBtnPressed);
  return true;
}
