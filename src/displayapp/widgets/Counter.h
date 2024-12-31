#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      class Counter {
      public:
        Counter(int min, int max, lv_font_t& font);

        void Create();
        void UpBtnPressed(lv_event_t event);
        void DownBtnPressed(lv_event_t event);
        void SetValue(int newValue);
        void HideControls();
        void ShowControls();
        void EnableTwelveHourMode();
        void EnableMonthMode();
        void EnableFastMode(int fastSkip);
        void SetMax(int newMax);
        void SetValueChangedEventCallback(void* userData, void (*handler)(void* userData));

        int GetValue() const {
          return value;
        }

        lv_obj_t* GetObject() const {
          return counterContainer;
        };

      private:
        void UpdateLabel();
        void (*ValueChangedHandler)(void* userData) = nullptr;

        lv_obj_t* counterContainer;
        lv_obj_t* upBtn;
        lv_obj_t* downBtn;
        lv_obj_t* number;
        lv_obj_t* upperLine;
        lv_obj_t* lowerLine;
        lv_point_t linePoints[2];
        int min;
        int max;
        int value;
        const int leadingZeroCount;
        bool twelveHourMode = false;
        bool monthMode = false;
        bool fastMode = false;
        int fastSkip = 1;
        lv_font_t& font;

        void* userData = nullptr;
      };
    }
  }
}
