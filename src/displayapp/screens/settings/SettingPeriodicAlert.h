#pragma once
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingPeriodicAlert : public Screen {
      public:
        SettingPeriodicAlert(Pinetime::Controllers::Settings& settingsController);
        ~SettingPeriodicAlert() override;

      private:
      };
    }
  }
}
