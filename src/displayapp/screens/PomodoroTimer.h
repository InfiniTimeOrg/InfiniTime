#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Symbols.h"
#include "utility/DirtyValue.h"
#include "components/pomodoro/PomodoroController.h"
#include <lvgl/lvgl.h>
#include <chrono>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class PomodoroTimer : public Screen {
      public:
        PomodoroTimer(AppControllers& controllers);
        ~PomodoroTimer() override;
        void Refresh() override;

      private:
        AppControllers& controllers;
        
        // Main UI components
        lv_obj_t* lblTime;
        lv_obj_t* lblSessionType;
        lv_obj_t* lblSessionProgress;
        lv_obj_t* btnStartPause;
        lv_obj_t* lblStartPause;
        lv_obj_t* btnSettings;
        lv_obj_t* lblSettings;
        
        // Progress indicators
        lv_obj_t* progressBar;
        lv_obj_t* lblDailyStats;
        lv_obj_t* lblCycleProgress;
        
        // Notification components
        lv_obj_t* notificationContainer;
        lv_obj_t* lblNotification;
        lv_task_t* taskHideNotification;
        
        // Settings modal components
        lv_obj_t* settingsModal;
        lv_obj_t* settingsContainer;
        lv_obj_t* btnWorkDuration25;
        lv_obj_t* btnWorkDuration50;
        lv_obj_t* lblShortBreakValue;
        lv_obj_t* btnShortBreakPlus;
        lv_obj_t* btnShortBreakMinus;
        lv_obj_t* lblLongBreakValue;
        lv_obj_t* btnLongBreakPlus;
        lv_obj_t* btnLongBreakMinus;
        lv_obj_t* lblSessionsValue;
        lv_obj_t* btnSessionsPlus;
        lv_obj_t* btnSessionsMinus;
        lv_obj_t* btnCloseSettings;
        bool settingsModalVisible;
        
        // Task for UI updates
        lv_task_t* taskRefresh;
        
        // State tracking for efficient updates
        Utility::DirtyValue<std::chrono::seconds> displaySeconds;
        Utility::DirtyValue<Controllers::PomodoroController::SessionState> displayState;
        Utility::DirtyValue<Controllers::PomodoroController::SessionType> displaySessionType;
        Utility::DirtyValue<uint8_t> displayCompletedSessions;
        Utility::DirtyValue<uint8_t> displayCyclePosition;
        
        // UI update methods
        void UpdateTimeDisplay();
        void UpdateSessionTypeDisplay();
        void UpdateProgressDisplay();
        void UpdateControlButtons();
        void UpdateStatisticsDisplay();
        void ApplySessionTheme();
        
        // Event handlers
        static void StartPauseEventHandler(lv_obj_t* obj, lv_event_t event);
        static void SettingsEventHandler(lv_obj_t* obj, lv_event_t event);
        
        // Helper methods
        const char* GetSessionTypeText(Controllers::PomodoroController::SessionType type);
        const char* GetButtonText(Controllers::PomodoroController::SessionState state);
        lv_color_t GetSessionThemeColor(Controllers::PomodoroController::SessionType type);
        lv_color_t GetSessionSecondaryColor(Controllers::PomodoroController::SessionType type);
        lv_color_t GetSessionBackgroundTint(Controllers::PomodoroController::SessionType type);
        int GetSessionDurationSeconds(Controllers::PomodoroController::SessionType type);
        void UpdateProgressBarColor(int remainingSeconds, int totalDuration);
        void HandleSessionCompletion();
        void ApplyPausedStateTheme();
        void ShowSessionCompletionNotification();
        void HideNotification();
        
        // Static callbacks
        static void HideNotificationCallback(lv_task_t* task);
        
        // Static callback for LVGL task
        static void RefreshTaskCallback(lv_task_t* task);
        
        // Settings modal methods
        void ShowSettingsModal();
        void HideSettingsModal();
        void UpdateSettingsDisplay();
        static void SettingsModalEventHandler(lv_obj_t* obj, lv_event_t event);
      };
    }

    template <>
    struct AppTraits<Apps::PomodoroTimer> {
      static constexpr Apps app = Apps::PomodoroTimer;
      static constexpr const char* icon = Screens::Symbols::clock;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::PomodoroTimer(controllers);
      }

      static bool IsAvailable(Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}