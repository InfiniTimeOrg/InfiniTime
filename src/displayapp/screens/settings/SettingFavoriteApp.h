#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Apps.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingFavoriteApp : public Screen {
      public:
        SettingFavoriteApp(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingFavoriteApp() override;

        bool Refresh() override;
        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;
        uint8_t optionsTotal;
        lv_obj_t* cbOption[13];
        Apps favoriteOptions[13] = {Apps::Music,Apps::Navigation,Apps::StopWatch,Apps::Timer,Apps::None,Apps::FlashLight,Apps::Paint,Apps::Paddle,Apps::Twos,Apps::HeartRate,Apps::Metronome,Apps::Motion,Apps::Steps};   
        char favoriteNames[13][12] = { "Music","Navigation","Stop Watch","Timer","None","Flash Light","Paint","Paddle","Twos","Heart Rate","Metronome","Motion","Steps"};
      };
    }
  }
}
