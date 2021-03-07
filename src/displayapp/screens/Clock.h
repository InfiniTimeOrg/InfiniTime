#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "Screen.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Controllers {
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
  }

  namespace Applications {
    namespace Screens {

      template <class T>
      class DirtyValue {
        public:
          DirtyValue() = default;                      // Use NSDMI
          explicit DirtyValue(T const& v):value{v}{}   // Use MIL and const-lvalue-ref
          bool IsUpdated() const { return isUpdated; }
          T const& Get() { this->isUpdated = false; return value; }  // never expose a non-const lvalue-ref
          DirtyValue& operator=(const T& other) {
            if (this->value != other) {
              this->value = other;
              this->isUpdated = true;
            }
            return *this;
          }
        private:
          T value{};            // NSDMI - default initialise type
          bool isUpdated{true}; // NSDMI - use brace initilisation
      };

      class Clock : public Screen {
        public:
          Clock(DisplayApp* app,
                  Controllers::DateTime& dateTimeController,
                  Controllers::Battery& batteryController,
                  Controllers::Ble& bleController,
                  Controllers::NotificationManager& notificatioManager,
                  Controllers::HeartRateController& heartRateController);
          ~Clock() override;

          bool Refresh() override;
          bool OnButtonPushed() override;

          void OnObjectEvent(lv_obj_t *pObj, lv_event_t i);
        private:
          char displayedChar[5];

          uint16_t currentYear = 1970;
          Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
          Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
          uint8_t currentDay = 0;

          DirtyValue<int> batteryPercentRemaining  {};
          DirtyValue<bool> bleState {};
          DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime{};
          DirtyValue<uint32_t> stepCount  {};
          DirtyValue<uint8_t> heartbeat  {};
          DirtyValue<bool> heartbeatRunning  {};
          DirtyValue<bool> notificationState {};

          lv_obj_t* label_time;
          lv_obj_t* label_date;
          lv_obj_t* backgroundLabel;
          lv_obj_t* batteryIcon;
          lv_obj_t* bleIcon;
          lv_obj_t* batteryPlug;
          lv_obj_t* heartbeatIcon;
          lv_obj_t* heartbeatValue;
          lv_obj_t* heartbeatBpm;
          lv_obj_t* stepIcon;
          lv_obj_t* stepValue;
          lv_obj_t* notificationIcon;

          Controllers::DateTime& dateTimeController;
          Controllers::Battery& batteryController;
          Controllers::Ble& bleController;
          Controllers::NotificationManager& notificatioManager;
          Controllers::HeartRateController& heartRateController;

          bool running = true;

      };
    }
  }
}
