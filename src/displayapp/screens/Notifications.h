#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <memory>
#include "Screen.h"
#include "components/ble/NotificationManager.h"
#include "components/motor/MotorController.h"

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
                               Pinetime::Controllers::MotorController& motorController,
                               Modes mode);
        ~Notifications() override;

        void Refresh() override;
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
          bool IsRunning() const {
            return running;
          }
          void OnCallButtonEvent(lv_obj_t*, lv_event_t event);

        private:
          lv_obj_t* container1;
          lv_obj_t* bt_accept;
          lv_obj_t* bt_mute;
          lv_obj_t* bt_reject;
          lv_obj_t* label_accept;
          lv_obj_t* label_mute;
          lv_obj_t* label_reject;
          Modes mode;
          Pinetime::Controllers::AlertNotificationService& alertNotificationService;
          bool running = true;
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
        lv_obj_t* timeoutLine = nullptr;
        uint32_t timeoutTickCountStart;
        uint32_t timeoutTickCountEnd;

        lv_task_t* taskRefresh;
      };
    }
  }
}
