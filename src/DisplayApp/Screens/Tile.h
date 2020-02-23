#pragma once

#include <cstdint>
#include <chrono>
#include <Components/Gfx/Gfx.h>
#include "Screen.h"
#include <bits/unique_ptr.h>
#include "../Fonts/lcdfont14.h"
#include "../Fonts/lcdfont70.h"
#include "../../Version.h"
#include <lvgl/src/lv_core/lv_style.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Tile : public Screen {
        public:
          explicit Tile(DisplayApp* app);
          ~Tile() override;

          bool Refresh(bool fullRefresh) override;
          bool OnButtonPushed() override;

          void OnObjectEvent(lv_obj_t* obj, lv_event_t event);

        private:

          lv_style_t* labelRelStyle;
          lv_style_t* labelPrStyle;
          lv_obj_t * label1;
          lv_obj_t * label2;
          lv_obj_t * label3;

          lv_obj_t* backgroundLabel;
          lv_obj_t * button;
          lv_obj_t * labelClick;

          lv_obj_t *tileview;
          lv_obj_t * tile1;
          lv_obj_t * tile2;
          lv_obj_t * list;
          lv_obj_t * list_btn;
          lv_obj_t * tile3;
          lv_obj_t * btn1;
          lv_obj_t * btn2;
          lv_obj_t * btn3;

          lv_obj_t * btnm1;
          lv_obj_t * btnm2;

          uint32_t clickCount = 0 ;
          uint32_t previousClickCount = 0;
          void StartApp();
          bool running = true;
      };
    }
  }
}
