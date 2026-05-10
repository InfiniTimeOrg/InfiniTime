#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/localization/Localization.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingWakeUp : public Screen {
      public:
        SettingWakeUp(Pinetime::Controllers::Settings& settingsController);
        ~SettingWakeUp() override;

        void UpdateSelected(lv_obj_t* object);

      private:
        struct Option {
          Controllers::Settings::WakeUpMode wakeUpMode;
          Localization::StringId name;
        };

        Controllers::Settings& settingsController;
        static constexpr std::array<Option, 5> options = {{
          {Controllers::Settings::WakeUpMode::SingleTap, Localization::StringId::SingleTap},
          {Controllers::Settings::WakeUpMode::DoubleTap, Localization::StringId::DoubleTap},
          {Controllers::Settings::WakeUpMode::RaiseWrist, Localization::StringId::RaiseWrist},
          {Controllers::Settings::WakeUpMode::Shake, Localization::StringId::ShakeWake},
          {Controllers::Settings::WakeUpMode::LowerWrist, Localization::StringId::LowerWrist},
        }};

        lv_obj_t* cbOption[options.size()];
      };
    }
  }
}
