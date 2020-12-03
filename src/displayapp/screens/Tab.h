#pragma once

#include <cstdint>
#include "Screen.h"
#include <bits/unique_ptr.h>
#include <lvgl/src/lv_core/lv_style.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Tab : public Screen {
        public:
          explicit Tab(DisplayApp* app, Components::Gfx& gfx);
          ~Tab() override;
          void Refresh(bool fullRefresh) override;
          void OnObjectEvent(lv_obj_t* obj, lv_event_t event);

        private:

      };
    }
  }
}
