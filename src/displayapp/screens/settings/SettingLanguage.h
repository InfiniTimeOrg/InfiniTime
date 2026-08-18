#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <memory>
#include "displayapp/screens/CheckboxList.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/localization/Localization.h"
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class SettingLanguage : public Screen {
      public:
        SettingLanguage(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingLanguage() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        static constexpr int languagesPerScreen = CheckboxList::MaxItems;
        static constexpr int nScreens = (static_cast<int>(Pinetime::Applications::Localization::Language::Count) - 1) / languagesPerScreen + 1;

        Pinetime::Controllers::Settings& settingsController;
        ScreenList<nScreens> screens;
      };

    }
  }
}
