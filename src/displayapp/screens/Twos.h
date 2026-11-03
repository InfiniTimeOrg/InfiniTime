#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"

namespace Pinetime {
  namespace Applications {
    struct TwosTile {
      bool merged = false;
      unsigned int value = 0;
    };

    namespace Screens {
      // Holds the state of a game of Twos and the rules that manipulate it.
      // Knows nothing about how the game is drawn.
      class TwosGrid {
      public:
        static constexpr int nCols = 4;
        static constexpr int nRows = 4;
        static constexpr int nCells = nCols * nRows;

        // Puts a new tile on a randomly chosen empty cell.
        // Returns false when there is no empty cell left, which means the game is lost.
        bool PlaceNewTile();

        // Slides every tile as far as it goes in the given direction, merging equal
        // tiles it runs into. rowStep and colStep give the direction, for example
        // {0, -1} to slide left and {1, 0} to slide down.
        // Returns true if any tile moved or merged.
        bool Slide(int rowStep, int colStep);

        unsigned int GetTileValue(int row, int col) const {
          return grid[row][col].value;
        }

        unsigned int GetScore() const {
          return score;
        }

      private:
        bool TryMerge(int newRow, int newCol, int oldRow, int oldCol);
        bool TryMove(int newRow, int newCol, int oldRow, int oldCol);
        void ResetMergeState();

        TwosTile grid[nRows][nCols];
        unsigned int score = 0;
      };

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
        TwosGrid grid;
        void UpdateGridDisplay();
        void UpdateScoreDisplay();
      };
    }

    template <>
    struct AppTraits<Apps::Twos> {
      static constexpr Apps app = Apps::Twos;
      static constexpr const char* icon = "2";

      static Screens::Screen* Create(AppControllers& /*controllers*/) {
        return new Screens::Twos();
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
