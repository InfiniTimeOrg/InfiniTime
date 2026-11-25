#include "displayapp/screens/Hiit.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

Hiit::Hiit(Controllers::MotorController& motorController, Controllers::Settings& settingsController, System::SystemTask& systemTask)
  : motorController {motorController}, settingsController {settingsController}, wakeLock(systemTask) {

  CreateSetupUI();

  taskRefresh = lv_task_create(Screen::RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Hiit::~Hiit() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());

  // Restore notification status if workout was in progress
  if (currentState != State::Setup && currentState != State::Summary) {
    settingsController.SetNotificationStatus(savedNotificationStatus);
  }
}

void Hiit::CreateSetupUI() {
  lv_obj_clean(lv_scr_act());

  // Active counter
  activeCounter.Create();
  activeCounter.SetValue(activeDuration);
  lv_obj_align(activeCounter.GetObject(), lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 6, -20);

  lv_obj_t* lblActive = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(lblActive, "ACTIVE");
  lv_obj_align(lblActive, activeCounter.GetObject(), LV_ALIGN_OUT_TOP_MID, 0, 0);

  // Rest counter
  restCounter.Create();
  restCounter.SetValue(restDuration);
  lv_obj_align(restCounter.GetObject(), lv_scr_act(), LV_ALIGN_CENTER, 0, -20);

  lv_obj_t* lblRest = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(lblRest, "REST");
  lv_obj_align(lblRest, restCounter.GetObject(), LV_ALIGN_OUT_TOP_MID, 0, 0);

  // Sets counter
  setsCounter.Create();
  setsCounter.SetValue(totalSets);
  lv_obj_align(setsCounter.GetObject(), lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -6, -20);

  lv_obj_t* lblSets = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(lblSets, "SETS");
  lv_obj_align(lblSets, setsCounter.GetObject(), LV_ALIGN_OUT_TOP_MID, 0, 0);

  // Start button
  btnStart = lv_btn_create(lv_scr_act(), nullptr);
  btnStart->user_data = this;
  lv_obj_set_event_cb(btnStart, OnStartClicked);
  lv_obj_set_size(btnStart, 120, 50);
  lv_obj_align(btnStart, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_style_local_bg_color(btnStart, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::green);

  lblStart = lv_label_create(btnStart, nullptr);
  lv_label_set_text_static(lblStart, Symbols::play);
  lv_obj_set_style_local_text_font(lblStart, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
}

void Hiit::CreateWorkoutUI() {
  lv_obj_clean(lv_scr_act());

  // Set info
  lblSetInfo = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblSetInfo, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_align(lblSetInfo, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, 10);

  // Phase label (ACTIVE/REST)
  lblPhase = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblPhase, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_align(lblPhase, lv_scr_act(), LV_ALIGN_CENTER, 0, -50);
  lv_obj_set_auto_realign(lblPhase, true);

  // Countdown timer
  lblCountdown = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblCountdown, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_align(lblCountdown, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_auto_realign(lblCountdown, true);

  // Total elapsed time
  lblTotalTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblTotalTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_label_set_align(lblTotalTime, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblTotalTime, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_auto_realign(lblTotalTime, true);
}

void Hiit::CreatePausedUI() {
  lv_obj_clean(lv_scr_act());

  // Set info
  lblSetInfo = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblSetInfo, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_label_set_text_fmt(lblSetInfo, "Set %d/%d", currentSet, totalSets);
  lv_obj_align(lblSetInfo, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, 10);

  // Paused label
  lv_obj_t* lblPaused = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(lblPaused, "PAUSED");
  lv_obj_set_style_local_text_font(lblPaused, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(lblPaused, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::orange);
  lv_obj_align(lblPaused, lv_scr_act(), LV_ALIGN_CENTER, 0, -50);

  // Frozen countdown
  lblCountdown = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblCountdown, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  uint8_t remaining = GetRemainingSeconds();
  lv_label_set_text_fmt(lblCountdown, "%d:%02d", remaining / 60, remaining % 60);
  lv_obj_align(lblCountdown, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  // Resume button
  btnResume = lv_btn_create(lv_scr_act(), nullptr);
  btnResume->user_data = this;
  lv_obj_set_event_cb(btnResume, OnResumeClicked);
  lv_obj_set_size(btnResume, 140, 50);
  lv_obj_align(btnResume, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_style_local_bg_color(btnResume, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::blue);

  lblResume = lv_label_create(btnResume, nullptr);
  lv_label_set_text_static(lblResume, "RESUME");
  lv_obj_set_style_local_text_font(lblResume, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
}

void Hiit::CreateSummaryUI(uint32_t finalTime) {
  lv_obj_clean(lv_scr_act());

  bool completed = (currentSet >= totalSets);

  // Status label
  lblStatus = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblStatus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);

  // Sets completed
  lblSetsCompleted = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblSetsCompleted, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);

  if (completed) {
    lv_label_set_text_static(lblStatus, "COMPLETE!");
    lv_obj_set_style_local_text_color(lblStatus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::green);
    lv_label_set_text_fmt(lblSetsCompleted, "%d", totalSets);
  } else {
    lv_label_set_text_static(lblStatus, "STOPPED");
    lv_obj_set_style_local_text_color(lblStatus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::orange);
    lv_label_set_text_fmt(lblSetsCompleted, "%d/%d", currentSet, totalSets);
  }
  lv_obj_align(lblStatus, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
  lv_obj_align(lblSetsCompleted, lv_scr_act(), LV_ALIGN_CENTER, 0, -48);

  lv_obj_t* lblSetsLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(lblSetsLabel, "Sets");
  lv_obj_set_style_local_text_font(lblSetsLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_align(lblSetsLabel, lblSetsCompleted, LV_ALIGN_OUT_TOP_MID, 0, 0);

  // Total time
  char timeBuffer[16];
  FormatTime(timeBuffer, finalTime);

  lblFinalTime = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(lblFinalTime, timeBuffer);
  lv_obj_set_style_local_text_font(lblFinalTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_align(lblFinalTime, lv_scr_act(), LV_ALIGN_CENTER, 0, 24);

  lv_obj_t* lblTimeLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(lblTimeLabel, "Total Time");
  lv_obj_set_style_local_text_font(lblTimeLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_align(lblTimeLabel, lblFinalTime, LV_ALIGN_OUT_TOP_MID, 0, 0);

  // Done button
  btnDone = lv_btn_create(lv_scr_act(), nullptr);
  btnDone->user_data = this;
  lv_obj_set_event_cb(btnDone, OnDoneClicked);
  lv_obj_set_size(btnDone, 120, 50);
  lv_obj_align(btnDone, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_style_local_bg_color(btnDone, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::blue);

  lblDone = lv_label_create(btnDone, nullptr);
  lv_label_set_text_static(lblDone, "DONE");
  lv_obj_set_style_local_text_font(lblDone, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
}

void Hiit::StartWorkout() {
  // Get config values from counters
  activeDuration = activeCounter.GetValue();
  restDuration = restCounter.GetValue();
  totalSets = setsCounter.GetValue();

  // Save and disable notifications during workout
  savedNotificationStatus = settingsController.GetNotificationStatus();
  settingsController.SetNotificationStatus(Controllers::Settings::Notification::Off);

  // Initialize state
  currentState = State::Starting;
  currentSet = 0;
  phaseStartTime = xTaskGetTickCount();
  workoutStartTime = 0; // Will be set after initial countdown

  wakeLock.Lock();

  CreateWorkoutUI();
}

void Hiit::TransitionToActive() {
  currentSet++;
  currentState = State::Active;
  phaseStartTime = xTaskGetTickCount();
  lastCountdownValue = 0;
}

void Hiit::TransitionToRest() {
  currentState = State::Rest;
  phaseStartTime = xTaskGetTickCount();
  lastCountdownValue = 0;
}

void Hiit::Pause() {
  stateBeforePause = currentState;
  currentState = State::Paused;
  pauseStartTime = xTaskGetTickCount();

  CreatePausedUI();
}

void Hiit::Resume() {
  // Calculate how long we were paused
  TickType_t pauseDuration = xTaskGetTickCount() - pauseStartTime;

  // Adjust start times to account for pause
  phaseStartTime += pauseDuration;
  workoutStartTime += pauseDuration;

  currentState = stateBeforePause;

  CreateWorkoutUI();
  UpdateWorkoutDisplay();
}

void Hiit::EndWorkout() {
  // Calculate elapsed time before changing state
  uint32_t finalTime = GetElapsedWorkoutTime();

  // Restore notification status
  settingsController.SetNotificationStatus(savedNotificationStatus);

  currentState = State::Summary;

  CreateSummaryUI(finalTime);
}

void Hiit::UpdateWorkoutDisplay() {
  // Update set info
  if (lblSetInfo != nullptr) {
    lv_label_set_text_fmt(lblSetInfo, "Set %d/%d", currentSet, totalSets);
  }

  // Update phase label
  if (lblPhase != nullptr) {
    if (currentState == State::Active) {
      lv_label_set_text_static(lblPhase, "ACTIVE");
      lv_obj_set_style_local_text_color(lblPhase, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::green);
    } else if (currentState == State::Rest) {
      lv_label_set_text_static(lblPhase, "REST");
      lv_obj_set_style_local_text_color(lblPhase, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::orange);
    }
  }

  // Update countdown
  if (lblCountdown != nullptr) {
    uint8_t remaining = GetRemainingSeconds();
    lv_label_set_text_fmt(lblCountdown, "%d:%02d", remaining / 60, remaining % 60);
  }

  // Update total time
  if (lblTotalTime != nullptr) {
    uint32_t totalSeconds = GetElapsedWorkoutTime();
    char timeBuffer[16];
    FormatTime(timeBuffer, totalSeconds);
    lv_label_set_text_fmt(lblTotalTime, "Total: %s", timeBuffer);
  }
}

void Hiit::Refresh() {
  switch (currentState) {
    case State::Setup:
    case State::Summary:
      // Nothing to update
      break;

    case State::Paused:
      // Frozen, nothing to update
      break;

    case State::Starting: {
      // Do a 3 second countdown before starting workout
      TickType_t elapsed = xTaskGetTickCount() - phaseStartTime;
      uint32_t elapsedSeconds = elapsed / configTICK_RATE_HZ;
      uint8_t remaining = (elapsedSeconds >= 3) ? 0 : (3 - elapsedSeconds);

      if (lblSetInfo != nullptr) {
        lv_label_set_text_static(lblSetInfo, "");
      }
      if (lblPhase != nullptr) {
        lv_label_set_text_static(lblPhase, "GET READY");
        lv_obj_set_style_local_text_color(lblPhase, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::orange);
      }
      if (lblCountdown != nullptr) {
        lv_label_set_text_fmt(lblCountdown, "%d", remaining);
      }
      if (lblTotalTime != nullptr) {
        lv_label_set_text_static(lblTotalTime, "");
      }

      // Countdown pulse
      if (remaining <= 3 && remaining > 0 && remaining != lastCountdownValue) {
        lastCountdownValue = remaining;
        motorController.RunForDuration(50);
      }

      // Start workout after countdown
      if (remaining == 0) {
        workoutStartTime = xTaskGetTickCount();
        motorController.RunForDuration(100);
        TransitionToActive();
      }
      break;
    }

    case State::Active:
    case State::Rest: {
      UpdateWorkoutDisplay();

      uint8_t remaining = GetRemainingSeconds();

      // Countdown pulse for last 3 seconds
      if (remaining <= 3 && remaining > 0 && remaining != lastCountdownValue) {
        lastCountdownValue = remaining;
        motorController.RunForDuration(50);
      }

      // Check for phase transition
      if (remaining == 0) {
        motorController.RunForDuration(100);
        if (currentState == State::Active) {
          // Check if this was the final active interval
          if (currentSet >= totalSets) {
            EndWorkout();
          } else {
            TransitionToRest();
          }
        } else {
          // End of rest period - always transition to next active
          TransitionToActive();
        }
      }
      break;
    }
  }
}

bool Hiit::OnButtonPushed() {
  if (currentState == State::Starting) {
    // Cancel initial countdown
    settingsController.SetNotificationStatus(savedNotificationStatus);
    wakeLock.Release();
    currentState = State::Setup;
    currentSet = 0;
    CreateSetupUI();
    return true;
  } else if (currentState == State::Active || currentState == State::Rest) {
    Pause();
    return true;
  } else if (currentState == State::Paused) {
    // Stop workout and show summary
    EndWorkout();
    return true;
  }
  return false;
}

bool Hiit::OnTouchEvent(TouchEvents event) {
  // Prevent closing the app during active workout phases
  return (currentState == State::Active || currentState == State::Rest) && event == TouchEvents::SwipeDown;
}

uint8_t Hiit::GetRemainingSeconds() const {
  TickType_t elapsed = xTaskGetTickCount() - phaseStartTime;
  bool isActivePhase = (currentState == State::Active || (currentState == State::Paused && stateBeforePause == State::Active));
  uint8_t phaseDuration = isActivePhase ? activeDuration : restDuration;
  uint32_t elapsedSeconds = elapsed / configTICK_RATE_HZ;

  if (elapsedSeconds >= phaseDuration) {
    return 0;
  }
  return phaseDuration - elapsedSeconds;
}

uint32_t Hiit::GetElapsedWorkoutTime() const {
  TickType_t now = xTaskGetTickCount();
  if (currentState == State::Paused) {
    now = pauseStartTime; // Stop counting at pause time
  }
  TickType_t elapsed = now - workoutStartTime;
  return elapsed / configTICK_RATE_HZ;
}

void Hiit::FormatTime(char* buffer, uint32_t seconds) const {
  uint32_t minutes = seconds / 60;
  uint32_t secs = seconds % 60;
  sprintf(buffer, "%02lu:%02lu", minutes, secs);
}

void Hiit::OnStartClicked(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    auto* screen = static_cast<Hiit*>(obj->user_data);
    screen->StartWorkout();
  }
}

void Hiit::OnResumeClicked(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    auto* screen = static_cast<Hiit*>(obj->user_data);
    screen->Resume();
  }
}

void Hiit::OnDoneClicked(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    auto* screen = static_cast<Hiit*>(obj->user_data);
    screen->wakeLock.Release();
    screen->currentState = State::Setup;
    screen->currentSet = 0;
    screen->CreateSetupUI();
  }
}
