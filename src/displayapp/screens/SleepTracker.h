#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "systemtask/SystemTask.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"

#include <vector>
#include <numeric>  // for accumulate
#include <cmath>    // for abs

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SleepTracker : public Screen {
      public:
        SleepTracker(Controllers::HeartRateController& HeartRateController, Controllers::DateTime& DateTimeController, Controllers::FS& fsController, System::SystemTask& systemTask);
        ~SleepTracker() override;

        void Refresh() override;

        void GetBPM();

        void ClearDataCSV(const char* filename);

        // Data Processing functions
        double ConvertToMinutes(int hours, int minutes, int seconds);
        // Get the moving average of BPM Values
        std::vector<double> MovingAverage(const std::vector<int>& bpm, int windowSize);
        // Detect the sleep regions
        std::vector<std::pair<double, double>> DetectSleepRegions(const std::vector<double>& bpmData, const std::vector<double>& time, double threshold);
        // Get the sleep info
        void GetSleepInfo(std::vector<std::tuple<int, int, int, int, int>> data);

        // Read IO
        std::vector<std::tuple<int, int, int, int, int>> ReadDataCSV(const char* fileName);

      private:
        Controllers::HeartRateController& heartRateController;
        Controllers::DateTime& dateTimeController;
        Controllers::FS& fsController;
        Pinetime::System::WakeLock wakeLock;

        // For File IO
        void WriteDataCSV(const char* fileName, const std::vector<std::tuple<int, int, int, int, int>>& data);

        int bpm = 0;
        int prevBpm = 0;
        int rollingBpm = 0;

        lv_obj_t* label_hr;

        lv_task_t* mainRefreshTask;
        lv_task_t* hrRefreshTask;
      };
    }
    
    template <>
    struct AppTraits<Apps::SleepTracker> {
      static constexpr Apps app = Apps::SleepTracker;
      static constexpr const char* icon = Screens::Symbols::bed;
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::SleepTracker(controllers.heartRateController, controllers.dateTimeController, controllers.filesystem, *controllers.systemTask);
      }
    };
  }
}