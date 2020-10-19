#pragma once

#include <cstdint>
#include "Screen.h"
#include <bits/unique_ptr.h>
#include "Modal.h"
#include <lvgl/src/lv_core/lv_style.h>
#include <displayapp/Apps.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Notifications2 : public Screen {
        public:
          explicit Notifications2(DisplayApp* app);
          ~Notifications2() override;

          bool Refresh() override;
          bool OnButtonPushed() override;
          void OnObjectEvent(lv_obj_t* obj, lv_event_t event, uint32_t buttonId);
          bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

        private:
          class ListWidget {
            public:
              ListWidget();
            private:
              lv_obj_t* page = nullptr;
          };

          bool running = true;
          ListWidget list;
      };
    }
  }
}
