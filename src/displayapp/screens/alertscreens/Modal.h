#pragma once

#include <cstdint>
#include <chrono>
#include "Screen.h"
#include <bits/unique_ptr.h>
#include <libs/lvgl/src/lv_core/lv_style.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      namespace Alert {
        class Modal : public Screen{
          public:
            Modal(DisplayApp* app);
            virtual ~Modal() override;

            virtual void Show(const char* msg) = 0;
            void Hide();

            bool Refresh() override;
            bool OnButtonPushed() override;

            static void mbox_event_cb(lv_obj_t *obj, lv_event_t evt);
          private:
            virtual void OnEvent(lv_obj_t *event_obj, lv_event_t evt) = 0;

            lv_style_t modal_style;
            lv_obj_t *obj;
            lv_obj_t *mbox;

            bool running = true;
            bool isVisible = false;

        };
      }
    }
  }
}
