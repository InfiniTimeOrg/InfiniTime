#pragma once

#include <lvgl/lvgl.h>
#include <FreeRTOS.h>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/ble/NotificationManager.h"
#include "components/motor/MotorController.h"
#include "systemtask/SystemTask.h"

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
                               System::SystemTask& systemTask,
                               Modes mode);
        ~Notifications() override;

        void Refresh() override;
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;
        void DismissToBlack();
        void OnPreviewInteraction();
        void OnPreviewDismiss();

        class NotificationItem {
        public:
          NotificationItem(Pinetime::Controllers::AlertNotificationService& alertNotificationService,
                           Pinetime::Controllers::MotorController& motorController);
          NotificationItem(const char* title,
                           const char* msg,
                           uint8_t notifNr,
                           Controllers::NotificationManager::Categories,
                           uint8_t notifNb,
                           Pinetime::Controllers::AlertNotificationService& alertNotificationService,
                           Pinetime::Controllers::MotorController& motorController);
          ~NotificationItem();

          bool IsRunning() const {
            return running;
          }

          void OnCallButtonEvent(lv_obj_t*, lv_event_t event);

        private:
          lv_obj_t* container;
          lv_obj_t* subject_container;
          lv_obj_t* bt_accept;
          lv_obj_t* bt_mute;
          lv_obj_t* bt_reject;
          lv_obj_t* label_accept;
          lv_obj_t* label_mute;
          lv_obj_t* label_reject;
          Pinetime::Controllers::AlertNotificationService& alertNotificationService;
          Pinetime::Controllers::MotorController& motorController;

          bool running = true;
        };

      private:
        DisplayApp* app;
        Pinetime::Controllers::NotificationManager& notificationManager;
        Pinetime::Controllers::AlertNotificationService& alertNotificationService;
        Pinetime::Controllers::MotorController& motorController;
        System::SystemTask& systemTask;
        Modes mode = Modes::Normal;
        std::unique_ptr<NotificationItem> currentItem;
        Pinetime::Controllers::NotificationManager::Notification::Id currentId;
        bool validDisplay = false;
        bool afterDismissNextMessageFromAbove = false;

        lv_point_t timeoutLinePoints[2] {{0, 1}, {239, 1}};
        lv_obj_t* timeoutLine = nullptr;
        TickType_t timeoutTickCountStart;

        static const TickType_t timeoutLength = pdMS_TO_TICKS(7000);
        bool interacted = true;

        bool dismissingNotification = false;

        lv_task_t* taskRefresh;
      };
    }
  }
}
