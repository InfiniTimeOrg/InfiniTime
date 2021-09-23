#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

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
        Id id;
        bool valid = false;
        uint8_t index;
        uint8_t size;
        std::array<char, MessageSize + 1> message;
        Categories category = Categories::Unknown;

        const char* Message() const;
        const char* Title() const;
      };
      Notification::Id nextId {0};

      void Push(Notification&& notif);
      Notification GetLastNotification();
      Notification GetNext(Notification::Id id);
      Notification GetPrevious(Notification::Id id);
      bool ClearNewNotificationFlag();
      bool AreNewNotificationsAvailable();

      static constexpr size_t MaximumMessageSize() {
        return MessageSize;
      };
      size_t NbNotifications() const;

    private:
      Notification::Id GetNextId();
      static constexpr uint8_t TotalNbNotifications = 5;
      std::array<Notification, TotalNbNotifications> notifications;
      uint8_t readIndex = 0;
      uint8_t writeIndex = 0;
      bool empty = true;
      std::atomic<bool> newNotification {false};
    };
  }
}
