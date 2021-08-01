#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <memory>
#include "Screen.h"
#include "components/ble/NotificationManager.h"

namespace Pinetime {
  namespace Controllers {
    class AlertNotificationService;
  }
  namespace Applications {
    namespace Screens {

      class Notifications : public Screen {
      public:
        enum class Modes { Normal, Preview };
        explicit Notifications(DisplayApp* app,
                               Pinetime::Controllers::NotificationManager& notificationManager,
                               Pinetime::Controllers::AlertNotificationService& alertNotificationService,
                               Modes mode);
        ~Notifications() override;

        bool Refresh() override;
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

        class NotificationItem {
        public:
          NotificationItem(const char* title,
                           const char* msg,
                           uint8_t notifNr,
                           Controllers::NotificationManager::Categories,
                           uint8_t notifNb,
                           Modes mode,
                           Pinetime::Controllers::AlertNotificationService& alertNotificationService);
          ~NotificationItem();
          bool Refresh() {
            return false;
          }
          void OnAcceptIncomingCall(lv_event_t event);
          void OnMuteIncomingCall(lv_event_t event);
          void OnRejectIncomingCall(lv_event_t event);

        private:
          uint8_t notifNr = 0;
          uint8_t notifNb = 0;
          char pageText[4];

          lv_obj_t* container1;
          lv_obj_t* t1;
          lv_obj_t* l1;
          lv_obj_t* l2;
          lv_obj_t* bt_accept;
          lv_obj_t* bt_mute;
          lv_obj_t* bt_reject;
          lv_obj_t* label_accept;
          lv_obj_t* label_mute;
          lv_obj_t* label_reject;
          lv_obj_t* bottomPlaceholder;
          Modes mode;
          Pinetime::Controllers::AlertNotificationService& alertNotificationService;
        };

      private:
        struct NotificationData {
          const char* title;
          const char* text;
        };
        Pinetime::Controllers::NotificationManager& notificationManager;
        Pinetime::Controllers::AlertNotificationService& alertNotificationService;
        Modes mode = Modes::Normal;
        std::unique_ptr<NotificationItem> currentItem;
        Controllers::NotificationManager::Notification::Id currentId;
        bool validDisplay = false;

        lv_point_t timeoutLinePoints[2] {{0, 1}, {239, 1}};
        lv_obj_t* timeoutLine;
        uint32_t timeoutTickCountStart;
        uint32_t timeoutTickCountEnd;
      };
    }
  }
}
