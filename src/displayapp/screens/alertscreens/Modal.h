#pragma once

#include <cstdint>
#include <chrono>

#include "displayapp/screens/Screen.h"
#include "components/ble/NotificationManager.h"
#include "displayapp/DisplayApp.h"

#include <bits/unique_ptr.h>
#include <libs/lvgl/src/lv_core/lv_style.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <libs/lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      namespace Alert {
        class Modal : public Screen{
          public:
            Modal(DisplayApp* app);
            virtual ~Modal() override;

            virtual void Show(struct Pinetime::Controllers::NotificationManager::Notification notification) = 0;
            void Hide();

            bool Refresh() override;
            bool OnButtonPushed() override;

            static void mbox_event_cb(lv_obj_t *obj, lv_event_t evt);
          private:
            virtual void OnEvent(lv_obj_t *event_obj, lv_event_t evt) = 0;

          protected:
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
