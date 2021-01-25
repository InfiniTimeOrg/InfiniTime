#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <memory>
#include "Screen.h"
#include "components/ble/NotificationManager.h"
#include "components/motor/MotorController.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Notifications : public Screen {
        public:
          enum class Modes {Normal, Preview};
          explicit Notifications(DisplayApp* app, 
                  Pinetime::Controllers::NotificationManager& notificationManager, 
                  Pinetime::Controllers::MotorController& motorController,
                  Modes mode);
          ~Notifications() override;

          bool Refresh() override;
          bool OnButtonPushed() override;
          bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

        private:
          bool running = true;

          class NotificationItem {
            public:
              NotificationItem(const char* title, const char* msg, uint8_t notifNr, uint8_t notifNb, Modes mode);
              ~NotificationItem();
              bool Refresh() {return false;}

            private:
              uint8_t notifNr = 0;
              uint8_t notifNb = 0;
              char pageText[4];

              lv_obj_t* container1;
              lv_obj_t* t1;
              lv_obj_t* l1;
              lv_obj_t* bottomPlaceholder;
              Modes mode;
          };

          struct NotificationData {
            const char* title;
            const char* text;
          };
          Pinetime::Controllers::NotificationManager& notificationManager;
          Pinetime::Controllers::MotorController& motorController;
          Modes mode = Modes::Normal;
          std::unique_ptr<NotificationItem> currentItem;
          Controllers::NotificationManager::Notification::Id currentId;
          bool validDisplay = false;

          lv_point_t timeoutLinePoints[2]  { {0, 237}, {239, 237} };
          lv_obj_t* timeoutLine;
          uint32_t timeoutTickCountStart;
          uint32_t timeoutTickCountEnd;
      };
    }
  }
}
