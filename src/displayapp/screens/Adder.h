#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include "components/fs/FS.h"
#include <list>
#include <vector>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // Save file version
      constexpr unsigned int AdderVersion = 1;

      struct AdderSave {
        unsigned int Level {0};
        unsigned int HighScore {0};
        unsigned int Version {AdderVersion};
      };

      enum class AdderField { UNDEFINED, BLANK, SOLID, BODY, FOOD };

      class Adder : public Screen {

      public:
        Adder(Pinetime::Components::LittleVgl& lvgl, Pinetime::Controllers::FS& fs);
        ~Adder() override;

        enum class MoveConsequence { DEATH, EAT, MOVE };

        // Overridden functions
        void Refresh() override;
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

      private:
        static constexpr const char* GameSavePath = "/games/adder/adder.sav";
        static constexpr unsigned int TileSize = 9;
        static constexpr unsigned int AdderDelayInterval = 200;

        Pinetime::Components::LittleVgl& lvgl;
        Controllers::FS& filesystem;

        AdderSave data; // Game save data
        AdderField* field {nullptr};

        lv_task_t* refreshTask {nullptr};
        lv_color_t* tileBuffer {nullptr};
        lv_color_t digitBuffer[64];

        unsigned int displayHeight {0};
        unsigned int displayWidth {0};
        unsigned int fieldWidth {0};
        unsigned int fieldHeight {0};
        unsigned int fieldSize {0};
        unsigned int highScore {2};

        unsigned int fieldOffsetHorizontal {0};
        unsigned int fieldOffsetVertical {0};

        std::list<unsigned int> adderBody;
        std::vector<unsigned int> blanks;

        int prevDirection {0};
        int currentDirection {1};

        bool appReady {false};

        // Methods
        void InitializeGame();
        void LoadGame();
        void SaveGame();
        void ResetGame();

        void InitializeBody();
        void CreateFood();
        void CreateLevel();
        
        void UpdatePosition();
        void FullRedraw();
        void UpdateSingleTile(unsigned int fieldX, unsigned int fieldY, lv_color_t color);
        void UpdateScore(unsigned int score);
        void GameOver();

        MoveConsequence CheckMove() const;
        
        static constexpr const char DigitFont[10][8] = {
          // Font for digits 0-9
          {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // 0
          {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // 1
          {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // 2
          {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // 3
          {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // 4
          {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // 5
          {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // 6
          {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // 7
          {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // 8
          {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}  // 9
        };
      };
    } // namespace Screens

    // Application Traits
    template <>
    struct AppTraits<Apps::Adder> {
      static constexpr Apps app = Apps::Adder;
      static constexpr const char* icon = "S";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Adder(controllers.lvgl, controllers.filesystem);
      }
    };

  } // namespace Applications
} // namespace Pinetime
