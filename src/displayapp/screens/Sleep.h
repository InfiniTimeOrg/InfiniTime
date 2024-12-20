#pragma once

#include "displayapp/apps/Apps.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/Counter.h"
#include "displayapp/widgets/PageIndicator.h"
#include "displayapp/Controllers.h"
#include "systemtask/SystemTask.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Sleep : public Screen {
      public:
        explicit Sleep(Controllers::InfiniSleepController& infiniSleepController,
                       Controllers::Settings::ClockType clockType,
                       System::SystemTask& systemTask,
                       Controllers::MotorController& motorController,
                       DisplayApp& displayApp);
        ~Sleep() override;
        void Refresh() override;
        void SetAlerting();
        void RedrawSetAlerting();
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
        bool OnButtonPushed() override;
        bool OnTouchEvent(TouchEvents event) override;
        void OnValueChanged();
        void StopAlerting(bool setSwitch = true);
        void SnoozeWakeAlarm();
        void UpdateDisplay();
        enum class SleepDisplayState { Info, Alarm, Settings };
        SleepDisplayState displayState = SleepDisplayState::Info;
        SleepDisplayState lastDisplayState = SleepDisplayState::Info;

        Controllers::InfiniSleepController& infiniSleepController;

        bool ignoreButtonPush = false;

        lv_obj_t *btnSnooze;

      private:
        System::WakeLock wakeLock;
        Controllers::MotorController& motorController;
        Controllers::Settings::ClockType clockType;
        DisplayApp& displayApp;

        lv_obj_t *btnStop, *txtStop, *txtSnooze, /**btnRecur, *txtRecur,*/ *btnInfo, *enableSwitch;
        lv_obj_t *trackerToggleBtn, *trackerToggleLabel;
        lv_obj_t* lblampm = nullptr;
        lv_obj_t* txtMessage = nullptr;
        lv_obj_t* btnMessage = nullptr;
        lv_task_t* taskSnoozeWakeAlarm = nullptr;

        lv_task_t* taskRefresh = nullptr;

        lv_task_t* taskPressesToStopAlarmTimeout = nullptr;

        // enum class EnableButtonState { On, Off, Alerting };
        void DisableWakeAlarm();
        void SetSwitchState(lv_anim_enable_t anim);
        void SetWakeAlarm();
        void UpdateWakeAlarmTime();
        Widgets::Counter hourCounter = Widgets::Counter(0, 23, jetbrains_mono_76);
        Widgets::Counter minuteCounter = Widgets::Counter(0, 59, jetbrains_mono_76);

        void DrawAlarmScreen();
        void DrawInfoScreen();
        void DrawSettingsScreen();
        bool StopAlarmPush();

        bool alreadyAlerting = false;

        lv_obj_t* label_hr;
        lv_obj_t* label_start_time;
        lv_obj_t* label_alarm_time;
        lv_obj_t* label_gradual_wake;
        lv_obj_t* label_total_sleep;
        lv_obj_t* label_sleep_cycles;
        lv_obj_t *btnSuggestedAlarm, *txtSuggestedAlarm, *iconSuggestedAlarm;

        lv_obj_t *lblWakeMode, *btnWakeMode, *lblWakeModeValue, *lblCycles, *btnCycles, *lblCycleValue, *btnTestMotorGradual,
          *lblMotorStrength, *btnMotorStrength, *lblMotorStrengthValue, *lblPushesToStop, *btnPushesToStop, *lblPushesToStopValue;

        Widgets::PageIndicator pageIndicator1 = Widgets::PageIndicator(0, 3);
        Widgets::PageIndicator pageIndicator2 = Widgets::PageIndicator(1, 3);
        Widgets::PageIndicator pageIndicator3 = Widgets::PageIndicator(2, 3);
      };
    }

    template <>
    struct AppTraits<Apps::Sleep> {
      static constexpr Apps app = Apps::Sleep;
      static constexpr const char* icon = Screens::Symbols::bed;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Sleep(controllers.infiniSleepController,
                                  controllers.settingsController.GetClockType(),
                                  *controllers.systemTask,
                                  controllers.motorController,
                                  *controllers.displayApp);
      }
    };
  }
}