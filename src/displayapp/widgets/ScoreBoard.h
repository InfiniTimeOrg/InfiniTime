#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      class ScoreBoard {
      public:
        ScoreBoard();
        int Width = 55;
        lv_obj_t* Create(char *title);

        void SetTopText(char *text);
        void SetBottomText(char *text);
        void SetTopText(uint8_t score);
        void SetBottomText(uint8_t score);

        void SetColor(lv_color_t color);
        lv_obj_t* GetObject() const {
          return counterContainer;
        };

      private:
        const int width = 50;
        const int titleHeight = 20;
        const int fullHeight = 130;
        lv_obj_t* topLabel;
        lv_obj_t* bottomLabel;
        lv_obj_t* topContainer;
        lv_obj_t* bottomContainer;
        lv_obj_t* counterContainer;

        void* userData = nullptr;
      };
    }
  }
}
