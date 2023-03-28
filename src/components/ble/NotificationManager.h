#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include "utility/StaticVector.h"

namespace Pinetime {
  namespace Controllers {
    class NotificationManager {
    public:
      enum class Categories {
        Unknown,
        SimpleAlert,
        Email,
        News,
        IncomingCall,
        MissedCall,
        Sms,
        VoiceMail,
        Schedule,
        HighProriotyAlert,
        InstantMessage
      };
      static constexpr uint8_t MessageSize {100};

      struct Notification {
        using Id = uint8_t;
        using Idx = uint8_t;
        Id id = 0;
        bool valid = false;
        uint8_t size;
        std::array<char, MessageSize + 1> message;
        Categories category = Categories::Unknown;

        const char* Message() const;
        const char* Title() const;
      };

      void Push(Notification&& notif);
      Notification GetNewestNotification() const;
      Notification Get(Notification::Id id) const;
      Notification GetNext(Notification::Id id) const;
      Notification GetPrevious(Notification::Id id) const;
      // Return the index of the notification with the specified id, if not found return NbNotifications()
      Notification::Idx NotificationPosition(Notification::Id id) const;
      bool ClearNewNotificationFlag();
      bool AreNewNotificationsAvailable() const;
      void Dismiss(Notification::Id id);

      static constexpr size_t MaximumMessageSize() {
        return MessageSize;
      };

      bool IsEmpty() const {
        return NbNotifications() == 0;
      }

      size_t NbNotifications() const {
        return notifications.Size();
      };

    private:
      Notification::Id nextId {0};
      Notification::Id GetNextId();
      void DismissIdx(Notification::Idx idx);
      Notification::Idx IndexOf(Notification::Id id) const;

      static constexpr uint8_t TotalNbNotifications = 5;
      Utility::StaticVector<Notification, TotalNbNotifications> notifications;

      std::atomic<bool> newNotification {false};
    };
  }
}
