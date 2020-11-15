#pragma once

#include "Screen.h"
#include <lvgl/src/lv_core/lv_style.h>
#include <lvgl/src/lv_core/lv_obj.h>

namespace Pinetime {
  namespace Components {
    class Gfx;
  }
  namespace Applications {
    namespace Screens {
      class Tab : public Screen {
        public:
          explicit Tab(DisplayApp* app, Components::Gfx& gfx);
          ~Tab() override;
          void Refresh(bool fullRefresh);
          void OnObjectEvent(lv_obj_t* obj, lv_event_t event);

        protected:
          Components::Gfx& gfx;
      };
    }
  }
}
