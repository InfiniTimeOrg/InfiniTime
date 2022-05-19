#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      class Counter {
      public:
        Counter(int min, int max);

        void Create();
        static void upBtnEventHandler(lv_obj_t* obj, lv_event_t event);
        static void downBtnEventHandler(lv_obj_t* obj, lv_event_t event);
        void Increment();
        void Decrement();
        void SetValue(int newValue);
        void HideControls();
        void ShowControls();

        int GetValue() const {
          return value;
        }

        lv_obj_t* GetObject() const {
          return counterContainer;
        };

      private:
        void UpdateLabel();

        lv_obj_t* counterContainer;
        lv_obj_t* upBtn;
        lv_obj_t* downBtn;
        lv_obj_t* number;
        lv_obj_t* upperLine;
        lv_obj_t* lowerLine;
        lv_point_t linePoints[2];
        int value = 0;
        int min;
        int max;
      };
    }
  }
}
