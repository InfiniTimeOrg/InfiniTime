#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <memory>
#include "Screen.h"
#include "../Apps.h"
#include "../DisplayApp.h"
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Tile : public Screen {
      public:
        struct Applications {
          const char* icon;
          Pinetime::Applications::Apps application;
        };

        explicit Tile(uint8_t screenID,
                      uint8_t numScreens,
                      DisplayApp* app,
                      Controllers::Settings& settingsController,
                      std::array<Applications, 6>& applications);

        ~Tile() override;

        void UpdateScreen();
        void OnValueChangedEvent(lv_obj_t* obj, uint32_t buttonId);

      private:
        lv_task_t* taskUpdate;

        DisplayApp::StatusBar statusBar;
        lv_point_t pageIndicatorBasePoints[2];
        lv_point_t pageIndicatorPoints[2];
        lv_obj_t* pageIndicatorBase;
        lv_obj_t* pageIndicator;
        lv_obj_t* btnm1;

        const char* btnmMap[8];
        Pinetime::Applications::Apps apps[6];
      };
    }
  }
}
