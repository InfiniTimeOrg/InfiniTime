#pragma once

#include <array>

namespace Pinetime {
  namespace Controllers {
    class NotificationManager {
      public:
        enum class Categories {Unknown, SimpleAlert, Email, News, IncomingCall, MissedCall, Sms, VoiceMail, Schedule, HighProriotyAlert, InstantMessage };
        static constexpr uint8_t MessageSize{18};

        struct Notification {
          std::array<char, MessageSize+1> message;
          Categories category = Categories::Unknown;
        };

      void Push(Categories category, const char* message, uint8_t messageSize);
      Notification Pop();


      private:
        static constexpr uint8_t TotalNbNotifications = 5;
        std::array<Notification, TotalNbNotifications> notifications;
        uint8_t readIndex = 0;
        uint8_t writeIndex = 0;
        bool empty = true;
    };
  }
}