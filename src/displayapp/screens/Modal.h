#pragma once

#include "Screen.h"
#include <lvgl/src/lv_core/lv_style.h>
#include <lvgl/src/lv_core/lv_obj.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Modal : public Screen{
        public:
          Modal(DisplayApp* app);
          ~Modal() override;

          void Show(const char* msg);
          void Hide();

          bool Refresh() override;
          bool OnButtonPushed() override;

          static void mbox_event_cb(lv_obj_t *obj, lv_event_t evt);
        private:
          void OnEvent(lv_obj_t *event_obj, lv_event_t evt);

          lv_style_t modal_style;
          lv_obj_t *obj;
          lv_obj_t *mbox;
          lv_obj_t *info;
          bool running = true;
          bool isVisible = false;

      };
    }
  }
}
