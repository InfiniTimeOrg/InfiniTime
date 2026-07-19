#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

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
      public:
        using Id = uint8_t;
        using Idx = uint8_t;

        Categories category = Categories::Unknown;
        Id id = 0;
        bool valid = false;

        Notification();
        Notification(const char* message, uint8_t size);
        Notification(const struct os_mbuf* om, int off, uint8_t size);
        const char* Message() const;
        const char* Title() const;

      private:
        std::array<char, MessageSize + 1> message {};
        uint8_t size;
      };

      void Push(Notification&& notif);
      Notification GetLastNotification() const;
      Notification Get(Notification::Id id) const;
      Notification GetNext(Notification::Id id) const;
      Notification GetPrevious(Notification::Id id) const;
      // Return the index of the notification with the specified id, if not found return NbNotifications()
      Notification::Idx IndexOf(Notification::Id id) const;
      bool ClearNewNotificationFlag();
      bool AreNewNotificationsAvailable() const;
      void Dismiss(Notification::Id id);

      static constexpr size_t MaximumMessageSize() {
        return MessageSize;
      };

      bool IsEmpty() const {
        return size == 0;
      }

      size_t NbNotifications() const;

    private:
      Notification::Id nextId {0};
      Notification::Id GetNextId();
      const Notification& At(Notification::Idx idx) const;
      Notification& At(Notification::Idx idx);
      void DismissIdx(Notification::Idx idx);

      static constexpr uint8_t TotalNbNotifications = 5;
      std::array<Notification, TotalNbNotifications> notifications;
      size_t beginIdx = TotalNbNotifications - 1; // index of the newest notification
      size_t size = 0;                            // number of valid notifications in buffer

      std::atomic<bool> newNotification {false};
    };
  }
}
