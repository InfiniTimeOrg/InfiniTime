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
    const int mins = (timeElapsedCentis / 100) / 60;
    return TimeSeparated_t {mins, secs, hundredths};
  }

  void PlayButtonEvent(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      stopWatch->PlayButtonEventHandler();
    }
  }

  void PauseButtonEvent(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      stopWatch->PauseButtonEventHandler();
    }
  }

  void StopButtonEvent(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      stopWatch->StopButtonEventHandler();
    }
  }

  void LapButtonEvent(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<StopWatch*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      stopWatch->LapButtonEventHandler();
    }
  }

  constexpr TickType_t blinkInterval = pdMS_TO_TICKS(1000);

  std::array<Pinetime::Applications::Widgets::MultiPurposeButton<2>::ButtonData, 2> playPauseButtonData = {
    {{PlayButtonEvent, Symbols::play, Colors::blue}, {PauseButtonEvent, Symbols::pause, Colors::bgAlt}},
  };
  std::array<Pinetime::Applications::Widgets::MultiPurposeButton<2>::ButtonData, 2> stopLapButtonData = {
    {{StopButtonEvent, Symbols::stop, LV_COLOR_RED}, {LapButtonEvent, Symbols::lapsFlag, Colors::bgAlt}},
  };
}

StopWatch::StopWatch(System::SystemTask& systemTask)
  : systemTask {systemTask}, btnPlayPause(playPauseButtonData, this), btnStopLap(stopLapButtonData, this) {
  static constexpr uint8_t btnWidth = 115;
  static constexpr uint8_t btnHeight = 80;
  lv_obj_set_size(btnPlayPause.GetObject(), btnWidth, btnHeight);
  lv_obj_align(btnPlayPause.GetObject(), lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  lv_obj_set_size(btnStopLap.GetObject(), btnWidth, btnHeight);
  lv_obj_align(btnStopLap.GetObject(), lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  btnStopLap.Disable();

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

  SetInterfaceStopped();

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

StopWatch::~StopWatch() {
  lv_task_del(taskRefresh);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void StopWatch::SetInterfacePaused() {
  btnPlayPause.SetActiveButton(0);
  btnStopLap.SetActiveButton(0);
}

void StopWatch::SetInterfaceRunning() {
  lv_obj_set_state(time, LV_STATE_DEFAULT);
  lv_obj_set_state(msecTime, LV_STATE_DEFAULT);
  btnPlayPause.SetActiveButton(1);
  btnStopLap.SetActiveButton(1);

  btnStopLap.Enable();
}

void StopWatch::SetInterfaceStopped() {
  lv_obj_set_state(time, LV_STATE_DISABLED);
  lv_obj_set_state(msecTime, LV_STATE_DISABLED);

  btnPlayPause.SetActiveButton(0);
  lv_label_set_text_static(time, "00:00");
  lv_label_set_text_static(msecTime, "00");

  lv_label_set_text_static(lapText, "");
  btnStopLap.SetActiveButton(1);
  btnStopLap.Disable();
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
  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

void StopWatch::Pause() {
  SetInterfacePaused();
  startTime = 0;
  // Store the current time elapsed in cache
  oldTimeElapsed = laps[lapsDone];
  blinkTime = xTaskGetTickCount() + blinkInterval;
  currentState = States::Halted;
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void StopWatch::Refresh() {
  if (currentState == States::Running) {
    laps[lapsDone] = oldTimeElapsed + xTaskGetTickCount() - startTime;

    TimeSeparated_t currentTimeSeparated = convertTicksToTimeSegments(laps[lapsDone]);
    lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
    lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.hundredths);
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

void StopWatch::PlayButtonEventHandler() {
  Start();
}

void StopWatch::PauseButtonEventHandler() {
  Pause();
}

void StopWatch::LapButtonEventHandler() {
  lv_label_set_text(lapText, "");
  lapsDone = std::min(lapsDone + 1, maxLapCount);
  for (int i = lapsDone - displayedLaps; i < lapsDone; i++) {
    if (i < 0) {
      lv_label_ins_text(lapText, LV_LABEL_POS_LAST, "\n");
      continue;
    }
    TimeSeparated_t times = convertTicksToTimeSegments(laps[i]);
    char buffer[16];
    sprintf(buffer, "#%2d   %2d:%02d.%02d\n", i + 1, times.mins, times.secs, times.hundredths);
    lv_label_ins_text(lapText, LV_LABEL_POS_LAST, buffer);
  }
}

void StopWatch::StopButtonEventHandler() {
  Reset();
}

bool StopWatch::OnButtonPushed() {
  if (currentState == States::Running) {
    Pause();
    return true;
  }
  return false;
}
