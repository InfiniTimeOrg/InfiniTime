#pragma once

#include "Modal.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      namespace Alert {
        class InstantMessageAlert : public Modal{
          public:
            InstantMessageAlert(DisplayApp* app);
            virtual ~InstantMessageAlert() override;

            virtual void Show(Pinetime::Controllers::NotificationManager notification);
          private:
            const std::string BTN_OK = "Ok";
            const std::string BTN_CANCEL = "Cancel";
            
            virtual void OnEvent(lv_obj_t *event_obj, lv_event_t evt);
        };
      }
    }
  }
}
