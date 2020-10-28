#pragma once

#include <cstdint>
#include <chrono>
#include "Screen.h"
#include "Modal.h"
#include <bits/unique_ptr.h>
#include <libs/lvgl/src/lv_core/lv_style.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      namespace Alert {
        class InstantMessageAlert : public Modal{
          public:
            InstantMessageAlert(DisplayApp* app);
            virtual ~InstantMessageAlert() override;

            virtual void Show(const char* msg);
          private:
            virtual void OnEvent(lv_obj_t *event_obj, lv_event_t evt);
        };
      }
    }
  }
}
