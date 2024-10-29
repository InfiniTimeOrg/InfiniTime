#pragma once

#include "displayapp/apps/Apps.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/Counter.h"
#include "displayapp/Controllers.h"
#include "systemtask/SystemTask.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"

//#include <vector>
//#include <numeric>  // for accumulate
//#include <cmath>    // for abs

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Sleep : public Screen {
      public:
        //explicit Sleep(Controllers::HeartRateController& HeartRateController, Controllers::DateTime& DateTimeController, Controllers::FS& fsController, System::SystemTask& systemTask);
        explicit Sleep(Controllers::InfiniSleepController& infiniSleepController, Controllers::Settings::ClockType clockType, System::SystemTask& systemTask, Controllers::MotorController& motorController);
        ~Sleep() override;
        void SetAlerting();
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
        bool OnButtonPushed() override;
        bool OnTouchEvent(TouchEvents event) override;
        void OnValueChanged();
        void StopAlerting();
        
        enum class SleepDisplayState { Alarm, Info, Settings };
        SleepDisplayState displayState = SleepDisplayState::Alarm;

        // void Refresh() override;

        // void GetBPM();

        // void ClearDataCSV(const char* filename) const;

        // // Data Processing functions
        // float ConvertToMinutes(int hours, int minutes, int seconds) const;
        // Get the moving average of BPM Values
        //std::vector<float> MovingAverage(const std::vector<int>& bpm, int windowSize) const;
        // Detect the sleep regions
        //std::vector<std::pair<float, float>> DetectSleepRegions(const std::vector<float>& bpmData, const std::vector<float>& time, float threshold) const;
        // Get the sleep info
        //void GetSleepInfo(const std::vector<std::tuple<int, int, int, int, int>>& data) const;

        // Read IO
        //std::vector<std::tuple<int, int, int, int, int>> ReadDataCSV(const char* fileName) const;

      private:
        Controllers::InfiniSleepController& infiniSleepController;
        System::WakeLock wakeLock;
        Controllers::MotorController& motorController;

        lv_obj_t *btnStop, *txtStop, *btnRecur, *txtRecur, *btnInfo, *enableSwitch;
        lv_obj_t* lblampm = nullptr;
        lv_obj_t* txtMessage = nullptr;
        lv_obj_t* btnMessage = nullptr;
        lv_task_t* taskStopWakeAlarm = nullptr;

        enum class EnableButtonState { On, Off, Alerting };
        void DisableWakeAlarm();
        void SetRecurButtonState();
        void SetSwitchState(lv_anim_enable_t anim);
        void SetWakeAlarm();
        void ShowAlarmInfo();
        void HideAlarmInfo();
        void ToggleRecurrence();
        void UpdateWakeAlarmTime();
        Widgets::Counter hourCounter = Widgets::Counter(0, 23, jetbrains_mono_76);
        Widgets::Counter minuteCounter = Widgets::Counter(0, 59, jetbrains_mono_76);

        // Controllers::HeartRateController& heartRateController;
        // Controllers::DateTime& dateTimeController;
        // Controllers::FS& fsController;
        // Pinetime::System::WakeLock wakeLock;

        // // For File IO
        // void WriteDataCSV(const char* fileName, const std::tuple<int, int, int, int, int>* data, int dataSize) const;

        // int bpm = 0;
        // int prevBpm = 0;
        // int rollingBpm = 0;

        // lv_obj_t* label_hr;

        // lv_task_t* mainRefreshTask;
        // lv_task_t* hrRefreshTask;
      };
    }
    
    template <>
    struct AppTraits<Apps::Sleep> {
      static constexpr Apps app = Apps::Sleep;
      static constexpr const char* icon = Screens::Symbols::bed;
      static Screens::Screen* Create(AppControllers& controllers) {
        //return new Screens::Sleep(controllers.heartRateController, controllers.dateTimeController, controllers.filesystem, *controllers.systemTask);
        return new Screens::Sleep(controllers.infiniSleepController, controllers.settingsController.GetClockType(), *controllers.systemTask, controllers.motorController);
      }
    };
  }
}