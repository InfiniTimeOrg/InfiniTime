#pragma once

#include "displayapp/screens/Screen.h"
#include "systemtask/SystemTask.h"
#include "displayapp/LittleVgl.h"
#include "displayapp/widgets/Counter.h"
#include "utility/DirtyValue.h"
#include <lvgl/lvgl.h>

#include "components/timer/Timer.h"
#include "Symbols.h"

namespace Pinetime::Applications {
  namespace Screens {
    class Timer : public Screen {
    public:
      Timer(Controllers::Timer& timerController, Controllers::MotorController& motorController);
      ~Timer() override;
      void Refresh() override;
      void Reset();
      void ToggleRunning();
      void ButtonPressed();
      void MaskReset();
      void SetTimerRinging();
      void OnLauncherButtonClicked(lv_obj_t* obj);

      bool launcherMode = true;

    private:
      void SetTimerRunning();
      void SetTimerStopped();
      void UpdateMask();
      void DisplayTime();
      void CreateLauncherUI();
      void CreateTimerUI(uint32_t startDurationMs, bool autoStart);
      void AddTimerDuration(uint32_t duration);
      uint32_t GetTimerDuration(uint8_t index) const;

      Pinetime::Controllers::Timer& timer;
      Pinetime::Controllers::MotorController& motorController;

      // Launcher UI elements
      static constexpr int numRecentTimers = 3;
      static uint32_t timerDurations[numRecentTimers];
      lv_obj_t* btnRecent[numRecentTimers] = {nullptr};
      lv_obj_t* btnCustom = nullptr;
      lv_obj_t* labelRecent[numRecentTimers] = {nullptr};
      lv_obj_t* labelCustom = nullptr;
      lv_style_t btnStyle;

      lv_obj_t* btnPlayPause;
      lv_obj_t* txtPlayPause;

      lv_obj_t* btnObjectMask;
      lv_obj_t* highlightObjectMask;
      lv_objmask_mask_t* btnMask;
      lv_objmask_mask_t* highlightMask;

      lv_task_t* taskRefresh;
      Widgets::Counter minuteCounter = Widgets::Counter(0, 59, jetbrains_mono_76);
      Widgets::Counter secondCounter = Widgets::Counter(0, 59, jetbrains_mono_76);

      bool buttonPressing = false;
      lv_coord_t maskPosition = 0;
      TickType_t pressTime = 0;
      Utility::DirtyValue<std::chrono::seconds> displaySeconds;
    };
  }

  template <>
  struct AppTraits<Apps::Timer> {
    static constexpr Apps app = Apps::Timer;
    static constexpr const char* icon = Screens::Symbols::hourGlass;

    static Screens::Screen* Create(AppControllers& controllers) {
      return new Screens::Timer(controllers.timer, controllers.motorController);
    };

    static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
      return true;
    };
  };
}
