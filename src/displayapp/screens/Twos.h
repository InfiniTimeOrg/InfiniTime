#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"

#include "displayapp/screens/Dice.h"

namespace Pinetime {
  namespace Applications {
    struct TwosTile {
      bool merged = false;
      unsigned int value = 0;
    };

    namespace Screens {
      class Twos : public Screen {
      public:
        Twos(Controllers::RNG& prngController);
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
        Controllers::RNG rng;
        unsigned int score = 0;
        void updateGridDisplay();
        bool tryMerge(int newRow, int newCol, int oldRow, int oldCol);
        bool tryMove(int newRow, int newCol, int oldRow, int oldCol);
        bool placeNewTile();
      };
    }

    template <>
    struct AppTraits<Apps::Twos> {
      static constexpr Apps app = Apps::Twos;
      static constexpr const char* icon = "2";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Twos(*controllers.prngController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
