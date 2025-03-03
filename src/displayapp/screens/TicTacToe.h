#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"

#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class TicTacToe : public Screen {
      public:
        enum class PlayerType : unsigned char { X = 'X', O = 'O', UNKNOWN = 0x00 };

        explicit TicTacToe();
        ~TicTacToe() override;

      private:
        lv_style_t cellStyle;
        lv_obj_t* playField;

        PlayerType currentPlayer = PlayerType::X;

        bool checkPath(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY, PlayerType to_check);
        bool checkDiagonal(PlayerType to_check);
        bool checkPlayer(PlayerType to_check);
        bool checkCellValue(uint16_t x, uint16_t y, PlayerType player);

        lv_obj_t* createLabel(int16_t x_offset);

        void showWin(PlayerType player);
        void showDraw();

        PlayerType hasWon();
        bool hasEnded();

        PlayerType charToPlayer(char inp) {
          switch (inp) {
            case 'X':
              return PlayerType::X;
            case 'O':
              return PlayerType::O;
            default:
              return PlayerType::UNKNOWN;
          }
        }

        static void pressCallback(lv_obj_t* obj, lv_event_t event);
      };
    }

    template <>
    struct AppTraits<Apps::TicTacToe> {
      static constexpr Apps app = Apps::TicTacToe;
      static constexpr const char* icon = Screens::Symbols::times;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::TicTacToe();
      }
    };
  }
}
