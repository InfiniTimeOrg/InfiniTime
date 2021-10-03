#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include "Screen.h"

namespace Pinetime {
  namespace Applications {
    struct Tile {
      bool merged = false;
      unsigned int value = 0;
    };
    namespace Screens {
      class Twos : public Screen {
      public:
        Twos(DisplayApp* app);
        ~Twos() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        lv_style_t style_cell1;
        lv_style_t style_cell2;
        lv_style_t style_cell3;
        lv_style_t style_cell4;
        lv_style_t style_cell5;

        lv_obj_t* scoreText;
        lv_obj_t* gridDisplay;
        Tile grid[4][4];
        unsigned int score = 0;
        void updateGridDisplay(Tile grid[][4]);
        bool tryMerge(Tile grid[][4], int& newRow, int& newCol, int oldRow, int oldCol);
        bool tryMove(Tile grid[][4], int newRow, int newCol, int oldRow, int oldCol);
        bool placeNewTile();
      };
    }
  }
}
