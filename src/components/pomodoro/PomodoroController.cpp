#include "components/pomodoro/PomodoroController.h"
#include "components/timer/Timer.h"
#include "components/datetime/DateTimeController.h"

using namespace Pinetime::Controllers;

PomodoroController::PomodoroController(Settings& settingsController, DateTime& dateTimeController) 
  : timer(this, TimerCallbackStatic),
    settingsController(settingsController),
    sessionStatistics(dateTimeController),
    currentState(SessionState::Ready),
    currentSessionType(SessionType::Work),
    pausedTimeRemaining(std::chrono::milliseconds::zero()),
    completedWorkSessions(0),
    sessionCompleteCallback(nullptr) {
}

void PomodoroController::StartWorkSession() {
  if (currentState != SessionState::Ready) {
    return;
  }
  
  currentSessionType = SessionType::Work;
  TransitionToActive();
  
  auto duration = GetSessionDuration(SessionType::Work);
  timer.StartTimer(duration);
}

void PomodoroController::StartBreak() {
  if (currentState != SessionState::Ready) {
    return;
  }
  
  currentSessionType = DetermineBreakType();
  TransitionToActive();
  
  auto duration = GetSessionDuration(currentSessionType);
  timer.StartTimer(duration);
}

void PomodoroController::PauseSession() {
  if (currentState != SessionState::Active) {
    return;
  }
  
  pausedTimeRemaining = timer.GetTimeRemaining();
  timer.StopTimer();
  TransitionToPaused();
}

void PomodoroController::ResumeSession() {
  if (currentState != SessionState::Paused) {
    return;
  }
  
  TransitionToActive();
  timer.StartTimer(pausedTimeRemaining);
  pausedTimeRemaining = std::chrono::milliseconds::zero();
}

void PomodoroController::CancelSession() {
  if (currentState == SessionState::Ready) {
    return;
  }
  
  timer.StopTimer();
  pausedTimeRemaining = std::chrono::milliseconds::zero();
  TransitionToReady();
}

PomodoroController::SessionState PomodoroController::GetCurrentState() const {
  return currentState;
}

PomodoroController::SessionType PomodoroController::GetCurrentSessionType() const {
  return currentSessionType;
}

std::chrono::milliseconds PomodoroController::GetTimeRemaining() {
  if (currentState == SessionState::Active) {
    return timer.GetTimeRemaining();
  } else if (currentState == SessionState::Paused) {
    return pausedTimeRemaining;
  }
  return std::chrono::milliseconds::zero();
}

uint8_t PomodoroController::GetCompletedWorkSessions() const {
  return completedWorkSessions;
}

uint8_t PomodoroController::GetCurrentCyclePosition() const {
  uint8_t sessionsBeforeLongBreak = settingsController.GetPomodoroSessionsBeforeLongBreak();
  return (completedWorkSessions % sessionsBeforeLongBreak) + 1;
}

bool PomodoroController::IsLongBreakDue() const {
  uint8_t sessionsBeforeLongBreak = settingsController.GetPomodoroSessionsBeforeLongBreak();
  return (completedWorkSessions > 0) && (completedWorkSessions % sessionsBeforeLongBreak == 0);
}

void PomodoroController::SetSessionCompleteCallback(TimerCallback callback) {
  sessionCompleteCallback = callback;
}

void PomodoroController::OnTimerCallback() {
  if (currentState != SessionState::Active) {
    return;
  }
  
  // Increment work session counter and update statistics if completing a work session
  if (currentSessionType == SessionType::Work) {
    completedWorkSessions++;
    sessionStatistics.IncrementCompletedSessions();
    
    // Add work time to daily statistics
    auto workDuration = GetSessionDuration(SessionType::Work);
    auto workMinutes = std::chrono::duration_cast<std::chrono::minutes>(workDuration);
    sessionStatistics.AddWorkTime(workMinutes);
  }
  
  TransitionToCompleted();
  
  // Trigger completion callback if registered
  if (sessionCompleteCallback) {
    sessionCompleteCallback();
  }
  
  // Transition back to ready state after completion
  TransitionToReady();
}

void PomodoroController::TransitionToReady() {
  currentState = SessionState::Ready;
}

void PomodoroController::TransitionToActive() {
  currentState = SessionState::Active;
}

void PomodoroController::TransitionToPaused() {
  currentState = SessionState::Paused;
}

void PomodoroController::TransitionToCompleted() {
  currentState = SessionState::Completed;
}

std::chrono::milliseconds PomodoroController::GetSessionDuration(SessionType type) const {
  switch (type) {
    case SessionType::Work: {
      auto workDuration = settingsController.GetPomodoroWorkDuration();
      return std::chrono::minutes(static_cast<uint8_t>(workDuration));
    }
    case SessionType::ShortBreak:
      return std::chrono::minutes(settingsController.GetPomodoroShortBreakMinutes());
    case SessionType::LongBreak:
      return std::chrono::minutes(settingsController.GetPomodoroLongBreakMinutes());
    default:
      return std::chrono::minutes(25);
  }
}

PomodoroController::SessionType PomodoroController::DetermineBreakType() const {
  return IsLongBreakDue() ? SessionType::LongBreak : SessionType::ShortBreak;
}

void PomodoroController::ResetSessionCounter() {
  completedWorkSessions = 0;
}

void PomodoroController::SetSessionsBeforeLongBreak(uint8_t sessions) {
  // This method is now deprecated since settings are managed by Settings controller
  // Keep for backward compatibility but don't use internal variable
  settingsController.SetPomodoroSessionsBeforeLongBreak(sessions);
}

uint8_t PomodoroController::GetDailyCompletedSessions() const {
  return sessionStatistics.GetDailyCompletedSessions();
}

std::chrono::minutes PomodoroController::GetDailyWorkTime() const {
  return sessionStatistics.GetDailyWorkTime();
}

void PomodoroController::CheckAndResetDailyStatistics() {
  sessionStatistics.CheckAndResetIfNewDay();
}

void PomodoroController::TimerCallbackStatic(TimerHandle_t xTimer) {
  auto* controller = static_cast<PomodoroController*>(pvTimerGetTimerID(xTimer));
  if (controller != nullptr) {
    controller->OnTimerCallback();
  }
}