#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingDoubleClick : public Screen {
      public:
        SettingDoubleClick(Pinetime::Controllers::Settings& settingsController);
        ~SettingDoubleClick() override;

      private:
        CheckboxList checkboxList;
      };
    }
  }
}
