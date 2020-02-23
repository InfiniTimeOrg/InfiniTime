#pragma once

#include <cstdint>
#include <chrono>
#include <Components/Gfx/Gfx.h>
#include "Screen.h"
#include <bits/unique_ptr.h>
#include <libs/lvgl/src/lv_core/lv_style.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <Components/Battery/BatteryController.h>
#include <Components/Ble/BleController.h>
#include "../Fonts/lcdfont14.h"
#include "../Fonts/lcdfont70.h"
#include "../../Version.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      template <class T>
      class DirtyValue {
        public:
          explicit DirtyValue(T v) { value = v; }
          explicit DirtyValue(T& v) { value = v; }
          bool IsUpdated() const { return isUpdated; }
          T& Get() { this->isUpdated = false; return value; }

          DirtyValue& operator=(const T& other) {
            if (this->value != other) {
              this->value = other;
              this->isUpdated = true;
            }
            return *this;
          }
        private:
          T value;
          bool isUpdated = true;
      };
      class Clock : public Screen{
        public:
          Clock(DisplayApp* app,
                  Controllers::DateTime& dateTimeController,
                  Controllers::Battery& batteryController,
                  Controllers::Ble& bleController);
          ~Clock() override;

          bool Refresh() override;
          bool OnButtonPushed() override;

          void OnObjectEvent(lv_obj_t *pObj, lv_event_t i);
        private:
          static const char* MonthToString(Pinetime::Controllers::DateTime::Months month);
          static const char* DayOfWeekToString(Pinetime::Controllers::DateTime::Days dayOfWeek);
          static char const *DaysString[];
          static char const *MonthsString[];

          char displayedChar[5];

          uint16_t currentYear = 1970;
          Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
          Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
          uint8_t currentDay = 0;

          DirtyValue<uint8_t> batteryPercentRemaining  {0};
          DirtyValue<bool> bleState {false};
          DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>> currentDateTime;
          DirtyValue<Pinetime::Version> version;

          lv_style_t* labelStyle;
          lv_style_t labelBigStyle;
          lv_obj_t * label_battery;

          lv_obj_t * label_ble;
          lv_obj_t* label_time;
          lv_obj_t* label_date;
          lv_obj_t* label_version;
          lv_obj_t* backgroundLabel;

          Controllers::DateTime& dateTimeController;
          Controllers::Battery& batteryController;
          Controllers::Ble& bleController;

          bool running = true;

      };
    }
  }
}
