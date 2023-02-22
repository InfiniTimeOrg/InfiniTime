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
        Twos();
        ~Twos() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        static constexpr int nColors = 5;
        lv_style_t cellStyles[nColors];

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
