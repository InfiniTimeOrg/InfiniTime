#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include "displayapp/screens/Screen.h"

namespace Pinetime {
  namespace Applications {
    struct TwosTile {
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
        static constexpr int nCols = 4;
        static constexpr int nRows = 4;
        static constexpr int nCells = nCols * nRows;
        TwosTile grid[nRows][nCols];
        unsigned int score = 0;
        void updateGridDisplay();
        bool tryMerge(int newRow, int newCol, int oldRow, int oldCol);
        bool tryMove(int newRow, int newCol, int oldRow, int oldCol);
        bool placeNewTile();
      };
    }
  }
}
