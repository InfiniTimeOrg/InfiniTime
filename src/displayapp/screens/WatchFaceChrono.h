#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Controllers {
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {
    class WatchFaceChrono : public Screen {
        public:
          WatchFaceChrono(DisplayApp* app,
                          Controllers::DateTime& dateTimeController,
                          Controllers::Battery& batteryController,
                          Controllers::Ble& bleController,
                          Controllers::NotificationManager& notificatioManager,
                          Controllers::Settings& settingsController,
                          Controllers::HeartRateController& heartRateController,
                          Controllers::MotionController& motionController);
          ~WatchFaceChrono() override;

        void Refresh() override;
        bool OnButtonPushed() override;
        bool OnTouchEvent(TouchEvents event) override;
        
        void OnObjectEvent(lv_obj_t *pObj, lv_event_t i);

        private:

          void start();
          void stop();
          void reset();
          float getCurrentTime();

          float currentTime = 0.0f;
          bool stopWatchRunning = false;
        
          std::chrono::system_clock::time_point startTime;

          static const char* MonthToString(Pinetime::Controllers::DateTime::Months month);
          static const char* DayOfWeekToString(Pinetime::Controllers::DateTime::Days dayOfWeek);

          char displayedChar[5];

          uint16_t currentYear = 1970;
          Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
          Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
          uint8_t currentDay = 0;

          DirtyValue<float> batteryPercentRemaining  {0};
          DirtyValue<bool> powerPresent {};
          DirtyValue<bool> bleState {};
          DirtyValue<bool> bleRadioEnabled {};
          DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
          DirtyValue<bool> motionSensorOk {};
          DirtyValue<uint32_t> stepCount  {};
          DirtyValue<uint8_t> heartbeat  {};
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
          Controllers::Settings& settingsController;
          Controllers::HeartRateController& heartRateController;
          Controllers::MotionController& motionController;

          bool running = true;

        template <class T>
        }

      //class DirtyValue {
        //public:
          //explicit DirtyValue(T v) { value = v; }
          //explicit DirtyValue(T& v) { value = v; }
          //bool IsUpdated() const { return isUpdated; }
          //T& Get() { this->isUpdated = false; return value; }

          //DirtyValue& operator=(const T& other) {
            //if (this->value != other) {
              //this->value = other;
              //this->isUpdated = true;
            //}
            //return *this;
          //}
        //private:
          //T value;
          //bool isUpdated = true;
        //};
      };
    }
  }
}
