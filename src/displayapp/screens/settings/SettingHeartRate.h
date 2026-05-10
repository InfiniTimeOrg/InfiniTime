#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include <optional>
#include <array>

#include "components/settings/Settings.h"
#include "displayapp/localization/Localization.h"
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
          Pinetime::Applications::Localization::StringId name;
        };

        Pinetime::Controllers::Settings& settingsController;

        static constexpr std::array<Option, 7> options = {{
          {.intervalInSeconds = std::nullopt, .name = Pinetime::Applications::Localization::StringId::Off},
          {.intervalInSeconds = 0, .name = Pinetime::Applications::Localization::StringId::Continuous},
          {.intervalInSeconds = 30, .name = Pinetime::Applications::Localization::StringId::Empty},
          {.intervalInSeconds = 60, .name = Pinetime::Applications::Localization::StringId::Empty},
          {.intervalInSeconds = 5 * 60, .name = Pinetime::Applications::Localization::StringId::Empty},
          {.intervalInSeconds = 10 * 60, .name = Pinetime::Applications::Localization::StringId::Empty},
          {.intervalInSeconds = 30 * 60, .name = Pinetime::Applications::Localization::StringId::Empty},
        }};

        lv_obj_t* cbOption[options.size()];
      };
    }
  }
}
