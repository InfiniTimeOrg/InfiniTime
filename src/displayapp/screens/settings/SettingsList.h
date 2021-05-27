#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <memory>
#include "displayapp/screens/List.h"
#include "displayapp/Apps.h"
#include "components/settings/Settings.h"


namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SettingsList : public List {
      public:
        
        
        explicit SettingsList(uint8_t screenID,
                              uint8_t numScreens,
                              DisplayApp* app,
                              Controllers::Settings& settingsController,
                              std::array<Item, 4>& applications);
        
        ~SettingsList() override;
        
        bool Refresh() override;
        
        void OnButtonEvent(lv_obj_t* object, lv_event_t event);
      
      private:
        Controllers::Settings& settingsController;
        Pinetime::Applications::Apps apps[MAXLISTITEMS];
        
        
      };
    }
  }
}
