#include "displayapp/screens/StopWatch.h"

#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  TimeSeparated_t convertTicksToTimeSegments(const TickType_t timeElapsed) {
    // Centiseconds
    const int timeElapsedCentis = timeElapsed * 100 / configTICK_RATE_HZ;

    const int hundredths = (timeElapsedCentis % 100);
    const int secs = (timeElapsedCentis / 100) % 60;
    const int mins = ((timeElapsedCentis / 100) / 60) % 60;
    const int hours = ((timeElapsedCentis / 100) / 60) / 60;
    return TimeSeparated_t {hours, mins, secs, hundredths};
  }

  void play_pause_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      stopWatch->playPauseBtnEventHandler();
    }
  }

  void stop_lap_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      stopWatch->stopLapBtnEventHandler();
    }
  }

  constexpr TickType_t blinkInterval = pdMS_TO_TICKS(1000);
}

StopWatch::StopWatch(System::SystemTask& systemTask) : wakeLock(systemTask) {
  static constexpr uint8_t btnWidth = 115;
  static constexpr uint8_t btnHeight = 80;
  static constexpr uint8_t lapLineHeight = 25;
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
  lv_obj_align(lapText, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -(btnHeight + lapLineHeight));

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_label_set_text_static(time, "00:00");
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DISABLED, Colors::lightGray);
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

  msecTime = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(msecTime, "00");
  lv_obj_set_style_local_text_color(msecTime, LV_LABEL_PART_MAIN, LV_STATE_DISABLED, Colors::lightGray);
  lv_obj_align(msecTime, time, LV_ALIGN_IN_TOP_MID, 0, -1);

  SetInterfaceStopped();

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

StopWatch::~StopWatch() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void StopWatch::SetInterfacePaused() {
  lv_obj_set_style_local_bg_color(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::blue);
  lv_label_set_text_static(txtPlayPause, Symbols::play);
  lv_label_set_text_static(txtStopLap, Symbols::stop);
}

void StopWatch::SetInterfaceRunning() {
  lv_obj_set_state(time, LV_STATE_DEFAULT);
  lv_obj_set_state(msecTime, LV_STATE_DEFAULT);
  lv_obj_set_style_local_bg_color(btnPlayPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_style_local_bg_color(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);

  lv_label_set_text_static(txtPlayPause, Symbols::pause);
  lv_label_set_text_static(txtStopLap, Symbols::lapsFlag);

  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
  lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT);
}

void StopWatch::SetInterfaceStopped() {
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

void StopWatch::Reset() {
  SetInterfaceStopped();
  currentState = States::Init;
  oldTimeElapsed = 0;
  lapsDone = 0;
}

void StopWatch::Start() {
  SetInterfaceRunning();
  startTime = xTaskGetTickCount();
  currentState = States::Running;
  wakeLock.Lock();

  // Add first lap automatically when starting
  if (lapsDone == 0) {
    lapsDone = 1;
    laps[0] = 0; // Current lap starts at 0
    updateLapDisplay();
  }
}

void StopWatch::updateLapDisplay() {
  std::string displayText;
  displayText.reserve(maxLapCount * 20);

  std::array<char, 32> buffer{};

  // Reverse loop to show newest laps first
  for (int i = lapsDone - 1; i >= std::max(0, lapsDone - displayedLaps); i--) {
    TimeSeparated_t times;
    if (i == lapsDone - 1 && currentState == States::Running) {
      // For current lap, calculate time since last lap
      TickType_t currentLapTime = xTaskGetTickCount() - startTime;
      if (i > 0) {
        currentLapTime += oldTimeElapsed - laps[i - 1];
      }
      times = convertTicksToTimeSegments(currentLapTime);
    } else {
      // For completed laps, show the lap duration
      TickType_t lapDuration = (i == 0) ? laps[i] : laps[i] - laps[i - 1];
      times = convertTicksToTimeSegments(lapDuration);
    }

    int written = (times.hours == 0)
      ? snprintf(buffer.data(),
                buffer.size(),
                "Lap %d    %2d:%02d.%02d\n",
                i + 1,
                times.mins,
                times.secs,
                times.hundredths)
      : snprintf(buffer.data(),
                buffer.size(),
                "Lap %d %2d:%02d:%02d.%02d\n",
                i + 1,
                times.hours,
                times.mins,
                times.secs,
                times.hundredths);

    if (written > 0 && written < static_cast<int>(buffer.size())) {
      displayText += buffer.data();
    }
  }

  lv_label_set_text(lapText, displayText.c_str());
}

void StopWatch::Pause() {
  SetInterfacePaused();
  startTime = 0;
  // Store the current time elapsed in cache
  oldTimeElapsed = laps[lapsDone];
  blinkTime = xTaskGetTickCount() + blinkInterval;
  currentState = States::Halted;
  wakeLock.Release();
}

void StopWatch::Refresh() {
  if (currentState == States::Running) {
    laps[lapsDone] = oldTimeElapsed + xTaskGetTickCount() - startTime;

    TimeSeparated_t currentTimeSeparated = convertTicksToTimeSegments(laps[lapsDone]);
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
    
    // Update the lap display
    updateLapDisplay();
  } else if (currentState == States::Halted) {
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
}

void StopWatch::playPauseBtnEventHandler() {
  if (currentState == States::Init || currentState == States::Halted) {
    Start();
  } else if (currentState == States::Running) {
    Pause();
  }
}

void StopWatch::stopLapBtnEventHandler() {
  // If running, then this button is used to save laps
  if (currentState == States::Running) {
    // Store the current lap time
    laps[lapsDone-1] = oldTimeElapsed + xTaskGetTickCount() - startTime;
    lapsDone = std::min(lapsDone + 1, maxLapCount);
    updateLapDisplay();
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