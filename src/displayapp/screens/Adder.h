#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "displayapp/screens/Screen.h"
#include "components/fs/FS.h"
#include <lvgl/lvgl.h>
#include <list>
#include <vector>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      const unsigned int AdderVersion = 1;

      struct AdderSave {
        unsigned int Level;
        unsigned int HighScore;
        unsigned int Version;

        AdderSave() : Level(0), HighScore(0),Version(AdderVersion){};
      };

      enum AdderField { UNDEFINED, BLANK, SOLID, PORTAL, BODY, FOOD };

      enum MoveConsequence { DEATH = 0, EAT, MOVE };

      class Adder : public Screen {

      public:
        Adder(Pinetime::Components::LittleVgl& lvgl, Pinetime::Controllers::FS& fs);
        ~Adder() override;
        void Refresh() override;
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;
        ;

      private: // Private members and methods

      const char* GameSavePath = "/games/adder/adder.sav";

        Controllers::FS& filesystem;
        AdderSave Data;
        const unsigned int TileSize = 9;
        const unsigned int AdderDelayInterval = 200;
        bool AppReady;
        unsigned int DisplayHeight;
        unsigned int DisplayWidth;

        unsigned int FieldWidth;
        unsigned int FieldHeight;
        unsigned int FieldSize;
        unsigned int FieldOffsetHorizontal;
        unsigned int FieldOffsetVertical;

        AdderField* Field;
        std::list<unsigned int> AdderBody;
        std::vector<unsigned int> Blanks;

        int prevAdderDirection = 0;
        int AdderDirection = 1;
        char lastKey = 0;

        unsigned int TileBufferSize;
        lv_color_t* TileBuffer;
        lv_color_t DigitBuffer[64];
        Pinetime::Components::LittleVgl& lvgl;
        lv_task_t* taskRefresh;

        unsigned int HighScore = 2;
        void createFood();
        void InitBody();

        MoveConsequence checkMove();
        void createLevel();
        void FullReDraw();
        void updateDisplay();
        void updateSingleTile(unsigned int FieldPosX, unsigned int FieldPosY, lv_color_t Color);
        void updatePosition();
        void updateScore(unsigned int Score);
        void GameOver();
        void LoadGame();
        void SaveGame();

        const char DigitFont[10][8] = {{0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},  // U+0030 (0)
                                       {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},  // U+0031 (1)
                                       {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},  // U+0032 (2)
                                       {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},  // U+0033 (3)
                                       {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},  // U+0034 (4)
                                       {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},  // U+0035 (5)
                                       {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},  // U+0036 (6)
                                       {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},  // U+0037 (7)
                                       {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},  // U+0038 (8)
                                       {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}}; // U+0039 (9)
      };
    }
      template <>
      struct AppTraits<Apps::Adder> {
      static constexpr Apps app = Apps::Adder;
      static constexpr const char* icon = "S";
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Adder(controllers.lvgl, controllers.filesystem);
      };
    };
  }
}