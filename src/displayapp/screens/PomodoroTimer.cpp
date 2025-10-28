#include "displayapp/screens/PomodoroTimer.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include "components/motor/MotorController.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

PomodoroTimer::PomodoroTimer(AppControllers& controllers) : controllers(controllers) {
  // Initialize dirty values
  displaySeconds = std::chrono::seconds {0};
  displayState = Controllers::PomodoroController::SessionState::Ready;
  displaySessionType = Controllers::PomodoroController::SessionType::Work;
  displayCompletedSessions = 0;
  displayCyclePosition = 0;
  displayDailyWorkTime = std::chrono::minutes {0};

  // Initialize notification components (initially hidden)
  notificationContainer = nullptr;
  lblNotification = nullptr;
  taskHideNotification = nullptr;

  // Create session type label at the top
  lblSessionType = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblSessionType, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(lblSessionType, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(lblSessionType, "Work Session");
  lv_obj_align(lblSessionType, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  // Create main time display (large, centered)
  lblTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_style_local_text_color(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(lblTime, "25:00");
  lv_obj_align(lblTime, lv_scr_act(), LV_ALIGN_CENTER, 0, -10);

  // Create progress bar for visual countdown
  progressBar = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_size(progressBar, 180, 6);
  lv_obj_align(progressBar, lblTime, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  lv_obj_set_style_local_bg_color(progressBar, LV_BAR_PART_BG, LV_STATE_DEFAULT, Colors::bgDark);
  lv_obj_set_style_local_bg_color(progressBar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Colors::highlight);
  lv_bar_set_range(progressBar, 0, 100);
  lv_bar_set_value(progressBar, 100, LV_ANIM_OFF);

  // Create session progress indicator
  lblSessionProgress = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lblSessionProgress, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(lblSessionProgress, "Session 1/4");
  lv_obj_align(lblSessionProgress, progressBar, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  // Create control buttons row
  btnStartPause = lv_btn_create(lv_scr_act(), nullptr);
  btnStartPause->user_data = this;
  lv_obj_set_size(btnStartPause, 80, 35);
  lv_obj_align(btnStartPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 15, -15);
  lv_obj_set_style_local_bg_color(btnStartPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
  lv_obj_set_event_cb(btnStartPause, StartPauseEventHandler);

  lblStartPause = lv_label_create(btnStartPause, nullptr);
  lv_label_set_text_static(lblStartPause, "START");
  lv_obj_align(lblStartPause, btnStartPause, LV_ALIGN_CENTER, 0, 0);

  // Create reset button
  btnReset = lv_btn_create(lv_scr_act(), nullptr);
  btnReset->user_data = this;
  lv_obj_set_size(btnReset, 80, 35);
  lv_obj_align(btnReset, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -15, -15);
  lv_obj_set_style_local_bg_color(btnReset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_event_cb(btnReset, ResetEventHandler);

  lblReset = lv_label_create(btnReset, nullptr);
  lv_label_set_text_static(lblReset, "RESET");
  lv_obj_align(lblReset, btnReset, LV_ALIGN_CENTER, 0, 0);

  // Create daily statistics display (positioned at top left - shows completed sessions)
  lblDailyStats = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lblDailyStats, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_obj_set_style_local_text_font(lblDailyStats, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_label_set_text_static(lblDailyStats, "0");
  lv_obj_align(lblDailyStats, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, 35);

  // Create cycle progress display (positioned at top right)
  lblCycleProgress = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lblCycleProgress, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(lblCycleProgress, "2/4");
  lv_obj_align(lblCycleProgress, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -10, 35);

  // Create daily work time display (positioned below daily stats)
  lblDailyWorkTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lblDailyWorkTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(lblDailyWorkTime, "0h 0m");
  lv_obj_align(lblDailyWorkTime, lblDailyStats, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);

  // Apply initial theme
  ApplySessionTheme();

  // Register session completion callback with PomodoroController
  controllers.pomodoroController.SetSessionCompleteCallback([this]() {
    ShowSessionCompletionNotification();
  });

  // Create refresh task for UI updates
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

PomodoroTimer::~PomodoroTimer() {
  // Clean up notification task if it exists
  if (taskHideNotification != nullptr) {
    lv_task_del(taskHideNotification);
  }

  // Clean up refresh task
  lv_task_del(taskRefresh);

  // Clear the session completion callback
  controllers.pomodoroController.SetSessionCompleteCallback(nullptr);

  lv_obj_clean(lv_scr_act());
}

void PomodoroTimer::Refresh() {
  auto& pomodoroController = controllers.pomodoroController;

  // Check for midnight reset (this will reset statistics if a new day has started)
  pomodoroController.CheckAndResetDailyStatistics();

  // Update time display - always update to ensure seconds are shown
  auto timeRemaining = pomodoroController.GetTimeRemaining();
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeRemaining);

  // Always update the display when timer is active to show seconds counting down
  if (pomodoroController.GetCurrentState() == Controllers::PomodoroController::SessionState::Active) {
    displaySeconds = seconds;
    UpdateTimeDisplay();
  } else {
    // When not active, only update if value changed
    displaySeconds = seconds;
    if (displaySeconds.IsUpdated()) {
      UpdateTimeDisplay();
    }
  }

  // Update session state
  auto currentState = pomodoroController.GetCurrentState();
  displayState = currentState;
  if (displayState.IsUpdated()) {
    UpdateControlButtons();

    // Handle session completion state
    if (currentState == Controllers::PomodoroController::SessionState::Completed) {
      // The notification is already triggered by the callback, just update UI
      UpdateSessionTypeDisplay();
      ApplySessionTheme();
    }
  }

  // Update session type
  auto sessionType = pomodoroController.GetCurrentSessionType();
  displaySessionType = sessionType;
  if (displaySessionType.IsUpdated()) {
    UpdateSessionTypeDisplay();
    ApplySessionTheme();
  }

  // Update session statistics
  auto completedSessions = pomodoroController.GetCompletedWorkSessions();
  displayCompletedSessions = completedSessions;
  if (displayCompletedSessions.IsUpdated()) {
    UpdateStatisticsDisplay();
  }

  auto cyclePosition = pomodoroController.GetCurrentCyclePosition();
  displayCyclePosition = cyclePosition;
  if (displayCyclePosition.IsUpdated()) {
    UpdateProgressDisplay();
  }

  // Update daily work time
  auto dailyWorkTime = pomodoroController.GetDailyWorkTime();
  displayDailyWorkTime = dailyWorkTime;
  if (displayDailyWorkTime.IsUpdated()) {
    UpdateStatisticsDisplay();
  }
}

void PomodoroTimer::UpdateTimeDisplay() {
  auto& pomodoroController = controllers.pomodoroController;
  auto timeRemaining = pomodoroController.GetTimeRemaining();

  auto totalSeconds = std::chrono::duration_cast<std::chrono::seconds>(timeRemaining).count();

  // Ensure we don't display negative time
  if (totalSeconds < 0) {
    totalSeconds = 0;
  }

  auto minutes = totalSeconds / 60;
  auto seconds = totalSeconds % 60;

  // Format time display with proper zero padding
  lv_label_set_text_fmt(lblTime, "%02ld:%02ld", minutes, seconds);

  // Update progress bar with session-specific durations
  // Get durations from settings or use defaults
  auto sessionType = pomodoroController.GetCurrentSessionType();
  int totalDuration = GetSessionDurationSeconds(sessionType);

  if (totalDuration > 0 && totalSeconds <= totalDuration) {
    // Calculate progress as percentage of time elapsed
    int progress = ((totalDuration - totalSeconds) * 100) / totalDuration;
    // Ensure progress is within valid range
    progress = std::max(0, std::min(100, progress));
    lv_bar_set_value(progressBar, progress, LV_ANIM_OFF);
  } else {
    // Reset progress bar if no valid session
    lv_bar_set_value(progressBar, 0, LV_ANIM_OFF);
  }

  // Update progress bar color based on remaining time
  UpdateProgressBarColor(totalSeconds, totalDuration);
}

void PomodoroTimer::UpdateSessionTypeDisplay() {
  auto& pomodoroController = controllers.pomodoroController;
  auto sessionType = pomodoroController.GetCurrentSessionType();

  lv_label_set_text_static(lblSessionType, GetSessionTypeText(sessionType));
}

void PomodoroTimer::UpdateProgressDisplay() {
  auto& pomodoroController = controllers.pomodoroController;
  auto cyclePosition = pomodoroController.GetCurrentCyclePosition();
  auto sessionsBeforeLongBreak = controllers.settingsController.GetPomodoroSessionsBeforeLongBreak();

  lv_label_set_text_fmt(lblSessionProgress, "Session %d/%d", cyclePosition, sessionsBeforeLongBreak);
  lv_label_set_text_fmt(lblCycleProgress, "%d/%d", cyclePosition, sessionsBeforeLongBreak);
}

void PomodoroTimer::UpdateControlButtons() {
  auto& pomodoroController = controllers.pomodoroController;
  auto currentState = pomodoroController.GetCurrentState();

  // Update button text
  lv_label_set_text_static(lblStartPause, GetButtonText(currentState));

  // Update button color based on state
  lv_color_t buttonColor;
  switch (currentState) {
    case Controllers::PomodoroController::SessionState::Ready:
      buttonColor = Colors::highlight; // Green for start
      break;
    case Controllers::PomodoroController::SessionState::Active:
      buttonColor = Colors::orange; // Orange for pause
      break;
    case Controllers::PomodoroController::SessionState::Paused:
      buttonColor = Colors::blue; // Blue for resume
      break;
    case Controllers::PomodoroController::SessionState::Completed:
      buttonColor = Colors::green; // Green for next
      break;
    default:
      buttonColor = Colors::bgAlt;
      break;
  }

  lv_obj_set_style_local_bg_color(btnStartPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, buttonColor);

  // Enable/disable button based on state
  bool buttonEnabled = true; // All states should allow button interaction
  lv_obj_set_click(btnStartPause, buttonEnabled);

  // Update reset button - enable when there's an active or paused session
  bool resetEnabled = (currentState == Controllers::PomodoroController::SessionState::Active ||
                       currentState == Controllers::PomodoroController::SessionState::Paused);
  lv_obj_set_click(btnReset, resetEnabled);

  // Visual feedback for disabled reset button
  lv_color_t resetColor = resetEnabled ? Colors::deepOrange : Colors::bgDark;
  lv_obj_set_style_local_bg_color(btnReset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, resetColor);
}

void PomodoroTimer::UpdateStatisticsDisplay() {
  auto& pomodoroController = controllers.pomodoroController;

  // Update daily completed sessions count
  auto dailyCompletedSessions = pomodoroController.GetDailyCompletedSessions();
  lv_label_set_text_fmt(lblDailyStats, "%d", dailyCompletedSessions);

  // Update daily work time display
  auto dailyWorkTime = pomodoroController.GetDailyWorkTime();
  auto hours = dailyWorkTime.count() / 60;
  auto minutes = dailyWorkTime.count() % 60;
  lv_label_set_text_fmt(lblDailyWorkTime, "%ldh %ldm", hours, minutes);
}

void PomodoroTimer::ApplySessionTheme() {
  auto& pomodoroController = controllers.pomodoroController;
  auto sessionType = pomodoroController.GetCurrentSessionType();
  auto currentState = pomodoroController.GetCurrentState();

  // Get theme colors for current session type
  auto primaryColor = GetSessionThemeColor(sessionType);
  auto secondaryColor = GetSessionSecondaryColor(sessionType);

  // Apply primary theme color to main elements
  lv_obj_set_style_local_text_color(lblSessionType, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, primaryColor);

  // Apply theme to progress bar with dynamic color based on remaining time
  auto timeRemaining = pomodoroController.GetTimeRemaining();
  auto totalSeconds = std::chrono::duration_cast<std::chrono::seconds>(timeRemaining).count();
  auto sessionDuration = GetSessionDurationSeconds(sessionType);
  UpdateProgressBarColor(totalSeconds, sessionDuration);

  // Apply theme to time display - make it more prominent during work sessions
  if (sessionType == Controllers::PomodoroController::SessionType::Work) {
    lv_obj_set_style_local_text_color(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    // Use consistent font size for better layout
    lv_obj_set_style_local_text_font(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  } else {
    // Softer appearance for break sessions
    lv_obj_set_style_local_text_color(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);
    lv_obj_set_style_local_text_font(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  }

  // Apply theme to session progress indicators
  lv_obj_set_style_local_text_color(lblSessionProgress, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);
  lv_obj_set_style_local_text_color(lblCycleProgress, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);
  lv_obj_set_style_local_text_color(lblDailyStats, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);

  // Apply paused state visual indicators if session is paused
  if (currentState == Controllers::PomodoroController::SessionState::Paused) {
    ApplyPausedStateTheme();
  }
}

const char* PomodoroTimer::GetSessionTypeText(Controllers::PomodoroController::SessionType type) {
  switch (type) {
    case Controllers::PomodoroController::SessionType::Work:
      return "Work Session";
    case Controllers::PomodoroController::SessionType::ShortBreak:
      return "Short Break";
    case Controllers::PomodoroController::SessionType::LongBreak:
      return "Long Break";
    default:
      return "Unknown";
  }
}

const char* PomodoroTimer::GetButtonText(Controllers::PomodoroController::SessionState state) {
  switch (state) {
    case Controllers::PomodoroController::SessionState::Ready:
      return "START";
    case Controllers::PomodoroController::SessionState::Active:
      return "PAUSE";
    case Controllers::PomodoroController::SessionState::Paused:
      return "RESUME";
    case Controllers::PomodoroController::SessionState::Completed:
      return "NEXT";
    default:
      return "START";
  }
}

lv_color_t PomodoroTimer::GetSessionThemeColor(Controllers::PomodoroController::SessionType type) {
  switch (type) {
    case Controllers::PomodoroController::SessionType::Work:
      return Colors::deepOrange; // Focus color for work - energetic and attention-grabbing
    case Controllers::PomodoroController::SessionType::ShortBreak:
      return Colors::green; // Rest color for short breaks - calming and refreshing
    case Controllers::PomodoroController::SessionType::LongBreak:
      return Colors::blue; // Different color for long breaks - deeper rest and recovery
    default:
      return Colors::highlight;
  }
}

lv_color_t PomodoroTimer::GetSessionSecondaryColor(Controllers::PomodoroController::SessionType type) {
  switch (type) {
    case Controllers::PomodoroController::SessionType::Work:
      return Colors::orange; // Softer orange for secondary elements during work
    case Controllers::PomodoroController::SessionType::ShortBreak:
      return LV_COLOR_MAKE(0x80, 0xd0, 0x80); // Light green for short break secondary elements
    case Controllers::PomodoroController::SessionType::LongBreak:
      return LV_COLOR_MAKE(0x80, 0xa0, 0xff); // Light blue for long break secondary elements
    default:
      return Colors::lightGray;
  }
}

void PomodoroTimer::StartPauseEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<PomodoroTimer*>(obj->user_data);

  // Handle button press visual feedback
  if (event == LV_EVENT_PRESSED) {
    // Darken button when pressed for visual feedback
    lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
    return;
  } else if (event == LV_EVENT_RELEASED || event == LV_EVENT_PRESS_LOST) {
    // Restore button opacity when released
    lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    return;
  }

  if (event != LV_EVENT_SHORT_CLICKED) {
    return;
  }

  auto& pomodoroController = screen->controllers.pomodoroController;
  auto currentState = pomodoroController.GetCurrentState();
  auto currentSessionType = pomodoroController.GetCurrentSessionType();

  switch (currentState) {
    case Controllers::PomodoroController::SessionState::Ready:
      // Start appropriate session type
      if (currentSessionType == Controllers::PomodoroController::SessionType::Work) {
        pomodoroController.StartWorkSession();
      } else {
        pomodoroController.StartBreak();
      }
      break;

    case Controllers::PomodoroController::SessionState::Active:
      pomodoroController.PauseSession();
      break;

    case Controllers::PomodoroController::SessionState::Paused:
      pomodoroController.ResumeSession();
      break;

    case Controllers::PomodoroController::SessionState::Completed:
      // Handle transition to next session type
      screen->HandleSessionCompletion();
      break;
  }

  // Immediately update the UI to reflect the state change
  screen->UpdateControlButtons();
}

void PomodoroTimer::HandleSessionCompletion() {
  UpdateSessionTypeDisplay();
  UpdateProgressDisplay();
  ApplySessionTheme();
}

void PomodoroTimer::ResetEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<PomodoroTimer*>(obj->user_data);

  // Handle button press visual feedback
  if (event == LV_EVENT_PRESSED) {
    lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
    return;
  } else if (event == LV_EVENT_RELEASED || event == LV_EVENT_PRESS_LOST) {
    lv_obj_set_style_local_bg_opa(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    return;
  }

  if (event != LV_EVENT_SHORT_CLICKED) {
    return;
  }

  // Reset current session (cancel and return to ready state)
  auto currentState = screen->controllers.pomodoroController.GetCurrentState();
  if (currentState == Controllers::PomodoroController::SessionState::Active ||
      currentState == Controllers::PomodoroController::SessionState::Paused) {
    screen->controllers.pomodoroController.CancelSession();
    screen->UpdateControlButtons();
    screen->UpdateTimeDisplay();
    screen->ApplySessionTheme();
  }
}

int PomodoroTimer::GetSessionDurationSeconds(Controllers::PomodoroController::SessionType type) {
  switch (type) {
    case Controllers::PomodoroController::SessionType::Work: {
      auto workDuration = controllers.settingsController.GetPomodoroWorkDuration();
      return static_cast<int>(workDuration) * 60;
    }
    case Controllers::PomodoroController::SessionType::ShortBreak:
      return controllers.settingsController.GetPomodoroShortBreakMinutes() * 60;
    case Controllers::PomodoroController::SessionType::LongBreak:
      return controllers.settingsController.GetPomodoroLongBreakMinutes() * 60;
    default:
      return 25 * 60;
  }
}

void PomodoroTimer::UpdateProgressBarColor(int remainingSeconds, int totalDuration) {
  if (totalDuration <= 0)
    return;

  // Change color based on remaining time percentage
  float remainingPercentage = (float) remainingSeconds / totalDuration;
  lv_color_t barColor;

  if (remainingPercentage > 0.5f) {
    // More than 50% remaining - use session theme color
    auto sessionType = controllers.pomodoroController.GetCurrentSessionType();
    barColor = GetSessionThemeColor(sessionType);
  } else if (remainingPercentage > 0.2f) {
    // 20-50% remaining - use orange warning
    barColor = Colors::orange;
  } else {
    // Less than 20% remaining - use red urgent
    barColor = Colors::deepOrange;
  }

  lv_obj_set_style_local_bg_color(progressBar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, barColor);
}

void PomodoroTimer::ApplyPausedStateTheme() {
  // Apply visual indicators for paused state
  // Dim the time display to indicate paused state
  lv_obj_set_style_local_text_color(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);

  // Add blinking effect to session type label to indicate paused state
  lv_obj_set_style_local_text_color(lblSessionType, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::orange);

  // Dim the progress bar
  lv_obj_set_style_local_bg_color(progressBar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Colors::lightGray);

  // Add "PAUSED" indicator to session type
  auto& pomodoroController = controllers.pomodoroController;
  auto sessionType = pomodoroController.GetCurrentSessionType();
  const char* sessionText = GetSessionTypeText(sessionType);

  // Create a paused indicator by modifying the session type text
  static char pausedText[64];
  snprintf(pausedText, sizeof(pausedText), "%s - PAUSED", sessionText);
  lv_label_set_text(lblSessionType, pausedText);
}

void PomodoroTimer::ShowSessionCompletionNotification() {
  auto& pomodoroController = controllers.pomodoroController;
  auto sessionType = pomodoroController.GetCurrentSessionType();

  // Trigger vibration feedback based on session type
  if (sessionType == Controllers::PomodoroController::SessionType::Work) {
    // Work session completed - longer vibration
    controllers.motorController.RunForDuration(90);
  } else {
    // Break completed - shorter vibration
    controllers.motorController.RunForDuration(60);
  }

  // Create notification container if it doesn't exist
  if (notificationContainer == nullptr) {
    notificationContainer = lv_cont_create(lv_scr_act(), nullptr);
    lv_obj_set_size(notificationContainer, 200, 60);
    lv_obj_align(notificationContainer, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(notificationContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_style_local_border_color(notificationContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, GetSessionThemeColor(sessionType));
    lv_obj_set_style_local_border_width(notificationContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_radius(notificationContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);

    lblNotification = lv_label_create(notificationContainer, nullptr);
    lv_obj_set_style_local_text_font(lblNotification, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
    lv_obj_set_style_local_text_color(lblNotification, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_align(lblNotification, notificationContainer, LV_ALIGN_CENTER, 0, 0);
  }

  // Set notification text based on session type
  const char* notificationText;
  if (sessionType == Controllers::PomodoroController::SessionType::Work) {
    notificationText = "Work Session\nCompleted!";
  } else if (sessionType == Controllers::PomodoroController::SessionType::ShortBreak) {
    notificationText = "Short Break\nCompleted!";
  } else {
    notificationText = "Long Break\nCompleted!";
  }

  lv_label_set_text(lblNotification, notificationText);
  lv_label_set_align(lblNotification, LV_LABEL_ALIGN_CENTER);

  // Update border color for current session type
  lv_obj_set_style_local_border_color(notificationContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, GetSessionThemeColor(sessionType));

  // Show the notification
  lv_obj_set_hidden(notificationContainer, false);

  // Create task to hide notification after 3 seconds
  if (taskHideNotification != nullptr) {
    lv_task_del(taskHideNotification);
  }
  taskHideNotification = lv_task_create(HideNotificationCallback, 3000, LV_TASK_PRIO_MID, this);
  lv_task_set_repeat_count(taskHideNotification, 1);
}

void PomodoroTimer::HideNotification() {
  if (notificationContainer != nullptr) {
    lv_obj_set_hidden(notificationContainer, true);
  }

  if (taskHideNotification != nullptr) {
    lv_task_del(taskHideNotification);
    taskHideNotification = nullptr;
  }
}

void PomodoroTimer::HideNotificationCallback(lv_task_t* task) {
  auto* screen = static_cast<PomodoroTimer*>(task->user_data);
  screen->HideNotification();
}

void PomodoroTimer::RefreshTaskCallback(lv_task_t* task) {
  auto* screen = static_cast<PomodoroTimer*>(task->user_data);
  screen->Refresh();
}