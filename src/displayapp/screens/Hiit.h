#pragma once

#include "displayapp/screens/Screen.h"
#include "components/motor/MotorController.h"
#include "components/settings/Settings.h"
#include "systemtask/SystemTask.h"
#include "systemtask/WakeLock.h"
#include "displayapp/LittleVgl.h"
#include "displayapp/widgets/Counter.h"
#include <lvgl/lvgl.h>
#include "Symbols.h"

namespace Pinetime::Applications {
  namespace Screens {
    class Hiit : public Screen {
    public:
      Hiit(Controllers::MotorController& motorController, Controllers::Settings& settingsController, System::SystemTask& systemTask);
      ~Hiit() override;
      void Refresh() override;
      bool OnButtonPushed() override;
      bool OnTouchEvent(TouchEvents event) override;

    private:
      enum class State { Setup, Starting, Active, Rest, Paused, Summary };

      // State management
      State currentState = State::Setup;
      State stateBeforePause = State::Active;

      // Configuration (in seconds)
      uint8_t activeDuration = 40;
      uint8_t restDuration = 20;
      uint8_t totalSets = 8;

      // Runtime state
      uint8_t currentSet = 0;
      TickType_t phaseStartTime = 0;
      TickType_t workoutStartTime = 0;
      TickType_t pauseStartTime = 0;
      uint8_t lastCountdownValue = 0;

      // Controllers
      Controllers::MotorController& motorController;
      Controllers::Settings& settingsController;
      System::WakeLock wakeLock;
      Controllers::Settings::Notification savedNotificationStatus = Controllers::Settings::Notification::On;

      // Setup UI elements
      Widgets::Counter activeCounter = Widgets::Counter(1, 99, jetbrains_mono_42);
      Widgets::Counter restCounter = Widgets::Counter(1, 99, jetbrains_mono_42);
      Widgets::Counter setsCounter = Widgets::Counter(1, 99, jetbrains_mono_42);
      lv_obj_t* btnStart = nullptr;
      lv_obj_t* lblStart = nullptr;

      // Workout UI elements
      lv_obj_t* lblSetInfo = nullptr;
      lv_obj_t* lblPhase = nullptr;
      lv_obj_t* lblCountdown = nullptr;
      lv_obj_t* lblTotalTime = nullptr;

      // Paused UI elements
      lv_obj_t* btnResume = nullptr;
      lv_obj_t* lblResume = nullptr;

      // Summary UI elements
      lv_obj_t* lblStatus = nullptr;
      lv_obj_t* lblSetsCompleted = nullptr;
      lv_obj_t* lblFinalTime = nullptr;
      lv_obj_t* btnDone = nullptr;
      lv_obj_t* lblDone = nullptr;

      // Task for refresh
      lv_task_t* taskRefresh = nullptr;

      // UI creation methods
      void CreateSetupUI();
      void CreateWorkoutUI();
      void CreatePausedUI();
      void CreateSummaryUI(uint32_t finalTime);

      // Transition methods
      void StartWorkout();
      void TransitionToActive();
      void TransitionToRest();
      void Pause();
      void Resume();
      void EndWorkout();

      // Update methods
      void UpdateWorkoutDisplay();

      // Utility
      uint8_t GetRemainingSeconds() const;
      uint32_t GetElapsedWorkoutTime() const;
      void FormatTime(char* buffer, uint32_t seconds) const;

      // Event handlers
      static void OnStartClicked(lv_obj_t* obj, lv_event_t event);
      static void OnResumeClicked(lv_obj_t* obj, lv_event_t event);
      static void OnDoneClicked(lv_obj_t* obj, lv_event_t event);
    };
  }

  template <>
  struct AppTraits<Apps::Hiit> {
    static constexpr Apps app = Apps::Hiit;
    static constexpr const char* icon = Screens::Symbols::dumbbell;

    static Screens::Screen* Create(AppControllers& controllers) {
      return new Screens::Hiit(controllers.motorController, controllers.settingsController, *controllers.systemTask);
    }

    static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
      return true;
    }
  };
}
