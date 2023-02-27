#pragma once
#include "components/settings/Settings.h"
#include "components/periodic_alert/PeriodicAlertController.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/Counter.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingPeriodicAlert : public Screen {
      public:
        SettingPeriodicAlert(Pinetime::Controllers::Settings& settingsController,
                             Pinetime::Controllers::PeriodicAlertController& controller);
        ~SettingPeriodicAlert() override;
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
        void OnValueChanged();

      private:
        void Update();

        Pinetime::Controllers::PeriodicAlertController& controller;
        Pinetime::Controllers::Settings& settingsController;
        lv_obj_t *enableSwitch, *helpText;
        Widgets::Counter minuteCounter = Widgets::Counter(0, 59, jetbrains_mono_76);
      };
    }
  }
}
