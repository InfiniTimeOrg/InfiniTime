#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingQuickAccess : public Screen {
      public:
        SettingQuickAccess(Pinetime::Controllers::Settings& settingsController);
        ~SettingQuickAccess() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        CheckboxList checkboxList;
      };
    }
  }
}
