#pragma once

#include "Modal.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      namespace Alert {
        class SimpleAlert : public Modal {
          public:
            SimpleAlert(DisplayApp* app);
            ~SimpleAlert() override;

            virtual void Show(Pinetime::Controllers::NotificationManager notification);
          private:
            const std::string BTN_OK = "Ok";
            
            virtual void OnEvent(lv_obj_t *event_obj, lv_event_t evt);
        }
      }
    }
  }
}
