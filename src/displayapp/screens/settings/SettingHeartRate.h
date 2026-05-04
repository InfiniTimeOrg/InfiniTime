#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include <optional>
#include <array>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {
      class SettingHeartRate : public Screen {
      public:
        explicit SettingHeartRate(Pinetime::Controllers::Settings& settings);
        ~SettingHeartRate() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        struct Option {
          std::optional<uint16_t> intervalInSeconds;
          const char* name;
        };

        Pinetime::Controllers::Settings& settingsController;

        static constexpr std::array<Option, 7> options = {{
          {.intervalInSeconds = std::nullopt, .name = " Off"},
          {.intervalInSeconds = 0, .name = "Cont"},
          {.intervalInSeconds = 30, .name = " 30s"},
          {.intervalInSeconds = 60, .name = "  1m"},
          {.intervalInSeconds = 5 * 60, .name = "  5m"},
          {.intervalInSeconds = 10 * 60, .name = " 10m"},
          {.intervalInSeconds = 30 * 60, .name = " 30m"},
        }};

        lv_obj_t* cbOption[options.size()];
      };
    }
  }
}
