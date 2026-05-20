#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingWidgets : public Screen {
      public:
        SettingWidgets(Pinetime::Controllers::Settings& settingsController);
        ~SettingWidgets() override;

        void UpdateSelected(lv_obj_t* object);

      private:
        struct Option {
          Controllers::Settings::Widget widget;
          const char* name;
        };

        Controllers::Settings& settingsController;
        static constexpr std::array<Option, 3> options = {{
          {Controllers::Settings::Widget::HeartRate, "Heart Rate"},
          {Controllers::Settings::Widget::Steps, "Steps"},
          {Controllers::Settings::Widget::Weather, "Weather"},
        }};

        lv_obj_t* cbOption[options.size()];
      };
    }
  }
}
