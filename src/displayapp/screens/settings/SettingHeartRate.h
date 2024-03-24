#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      struct Option {
        const Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval interval;
        const char* name;
      };

      class SettingHeartRate : public Screen {
      public:
        SettingHeartRate(Pinetime::Controllers::Settings& settings);
        ~SettingHeartRate() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Pinetime::Controllers::Settings& settingsController;

        static constexpr std::array<Option, 8> options = {{
          {Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::Off, " Off"},
          {Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::Continuous, "Cont"},
          {Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::TenSeconds, " 10s"},
          {Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::ThirtySeconds, " 30s"},
          {Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::OneMinute, "  1m"},
          {Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::FiveMinutes, "  5m"},
          {Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::TenMinutes, " 10m"},
          {Pinetime::Controllers::Settings::HeartRateBackgroundMeasurementInterval::ThirtyMinutes, " 30m"},
        }};

        lv_obj_t* cbOption[options.size()];
      };
    }
  }
}
