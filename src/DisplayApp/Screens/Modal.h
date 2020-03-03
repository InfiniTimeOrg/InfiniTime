#pragma once

#include <cstdint>
#include <chrono>
#include <Components/Gfx/Gfx.h>
#include "Screen.h"
#include <bits/unique_ptr.h>
#include <libs/lvgl/src/lv_core/lv_style.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <Components/Battery/BatteryController.h>
#include <Components/Ble/BleController.h>
#include "../Fonts/lcdfont14.h"
#include "../Fonts/lcdfont70.h"
#include "../../Version.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Modal : public Screen{
        public:
          Modal(DisplayApp* app);
          ~Modal() override;

          void Show();
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

      };
    }
  }
}
