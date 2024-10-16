#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/datetime/DateTimeController.h"
#include "utility/DirtyValue.h"
#include "displayapp/LittleVgl.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {


      // really just an abstraction of a uint8_t but with functions to get the individual bits
      // reading up on bitfields and their inconsistencies scared me away from them...
      struct MazeTile {
        static constexpr uint8_t UPMASK =    0b0001;
        static constexpr uint8_t LEFTMASK =  0b0010;
        static constexpr uint8_t FLAGEMPTYMASK = 0b0100;
        static constexpr uint8_t FLAGGENMASK = 0b1000;
        uint8_t map = 0;
        // set flags on tiles in the maze. returns the object so they can be chained.
        MazeTile setUp(bool value)    {map = (map&~UPMASK)    | (value * UPMASK);    return *this;}
        MazeTile setLeft(bool value)  {map = (map&~LEFTMASK)  | (value * LEFTMASK);  return *this;}
        MazeTile setFlagEmpty(bool value) {map = (map&~FLAGEMPTYMASK) | (value * FLAGEMPTYMASK); return *this;}
        MazeTile setFlagGen(bool value) {map = (map&~FLAGGENMASK) | (value * FLAGGENMASK); return *this;}
        // get flags on tiles in the maze
        bool getUp()    {return map&UPMASK;}
        bool getLeft()  {return map&LEFTMASK;}
        bool getFlagEmpty() {return map& FLAGEMPTYMASK;}
        bool getFlagGen() {return map& FLAGGENMASK;}
      };




      // custom PRNG for the maze to allow it to be deterministic for any given minute
      class MazeRNG {
      public:
        MazeRNG(uint64_t start_seed = 64) {seed(start_seed);}
        // reseed the generator. handles any input well. if it's 0, acts as though it was seeded with 1
        void seed(uint64_t seed) {state = seed ? seed : 1; rand();}
        // rng lifted straight from https://en.wikipedia.org/wiki/Xorshift#xorshift* (asterisk is part of the link)
        uint32_t rand() {
          state ^= state >> 12;
          state ^= state << 25;
          state ^= state >> 27;
          // wikipedia had the multiplication in the return. idk what it's doing exactly so i won't change that.
          return state * 0x2545F4914F6CDD1DULL >> 32;
        }
        // random in range, inclusive on both ends (don't make max<min)
        uint32_t rand(uint32_t min, uint32_t max) {assert(max>=min); return rand()%(max-min+1)+min;}
      private:
        uint64_t state;
      };




      // little bit of convenience
      enum TileAttr {up, down, left, right, flagempty, flaggen};

      // could also be called Field or something. Does not handle stuff like generation or printing,
      // ONLY handles interacting with the board.
      class Maze {
      public:
        Maze();
        // I guess I don't need an explicit destructor since everything goes out of scope nicely?
        // get and set can work with either xy or indexes
        MazeTile get(int x, int y);
        MazeTile get(int index);
        void set(int x, int y, MazeTile tile);
        void set(int index, MazeTile tile);
        // fill fills all tiles in the maze with a given value, optionally with a mask on what bits to mask
        // (use the MazeTile::*MASK-s)
        void fill(MazeTile tile, uint8_t mask = 0xFF);
        void fill(uint8_t value, uint8_t mask = 0xFF);
        // allows abstractly setting a given side on a tile. supports down and right for convenience.
        void setSide(int x, int y, TileAttr attr, bool value);
        void setSide(int index, TileAttr attr, bool value);
        // same as setSide, just getting.
        bool getSide(int x, int y, TileAttr attr);
        bool getSide(int index, TileAttr attr);
        // paste onto board with transparency: if empty flag is set, ignore the tile
        // toPaste is a 1d array of uint8_t, reflecting internal maze structure (so packed values)
        // always places values left to right, top to bottom. coords are inclusive
        // IS A NAIVE PASTE, X COORDS MUST BE COMPRESSED 2X IN WIDTH AND CONTENTS MUST BE PADDED FOR OFFSET PASTING
        void transparentPaste(int x1, int y1, int x2, int y2, const uint8_t toPaste[]);

        // 10x10 px tiles on the maze = 24x24 (on 240px screen)
        static constexpr int WIDTH = 24;
        static constexpr int HEIGHT = 24;
        static constexpr int TILESIZE = 10;
        // the actual size of the entire map. only store 4 bits per block, so 2 blocks per byte
        static constexpr int FLATSIZE = WIDTH*HEIGHT/2;
      private:
        uint8_t mazemap[FLATSIZE];
      };




      class WatchFaceMaze : public Screen {
      public:
        WatchFaceMaze(Pinetime::Components::LittleVgl&, Controllers::DateTime&, Controllers::Settings&);
        ~WatchFaceMaze() override;
	void Refresh() override;

      private:
        lv_task_t* taskRefresh;
        void GenerateMaze();
        void InitializeMaze();
        void PutNumbers();
        void DrawMaze();

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;
        Pinetime::Components::LittleVgl& lvgl;
        lv_color_t buf1[480];
        lv_color_t buf2[480];
        Maze maze;
        MazeRNG prng;
        bool pausedGeneration = false;  // if generation was taking too much time and had to pause it to let other tasks run
        uint8_t numbers[10][30] = {{0x73,0x11,0x17,0x30,0x00,0x01,0x20,0x77,0x20,0x20,0x77,0x20,0x20,0x77,0x20,0x20,0x77,0x20,0x20,0x77,0x20,0x20,0x77,0x20,0x20,0x11,0x00,0x72,0x00,0x07},
                                   {0x73,0x11,0x77,0x30,0x00,0x77,0x20,0x00,0x77,0x77,0x20,0x77,0x77,0x20,0x77,0x77,0x20,0x77,0x77,0x20,0x77,0x77,0x20,0x77,0x31,0x00,0x11,0x20,0x00,0x00},
                                   {0x73,0x11,0x17,0x30,0x00,0x01,0x20,0x77,0x20,0x20,0x77,0x20,0x77,0x73,0x00,0x77,0x30,0x00,0x73,0x00,0x07,0x30,0x00,0x77,0x20,0x00,0x11,0x20,0x00,0x00},
                                   {0x73,0x11,0x17,0x30,0x00,0x01,0x20,0x77,0x20,0x77,0x77,0x20,0x77,0x31,0x00,0x77,0x20,0x00,0x77,0x77,0x20,0x31,0x77,0x20,0x20,0x11,0x00,0x72,0x00,0x07},
                                   {0x31,0x77,0x31,0x20,0x77,0x20,0x20,0x77,0x20,0x20,0x77,0x20,0x20,0x11,0x00,0x20,0x00,0x00,0x77,0x77,0x20,0x77,0x77,0x20,0x77,0x77,0x20,0x77,0x77,0x20},
                                   {0x31,0x11,0x11,0x20,0x00,0x00,0x20,0x77,0x77,0x20,0x77,0x77,0x20,0x11,0x17,0x20,0x00,0x01,0x77,0x77,0x20,0x77,0x77,0x20,0x31,0x11,0x00,0x20,0x00,0x07},
                                   {0x73,0x11,0x17,0x30,0x00,0x07,0x20,0x77,0x77,0x20,0x77,0x77,0x20,0x11,0x17,0x20,0x00,0x01,0x20,0x77,0x20,0x20,0x77,0x20,0x20,0x11,0x00,0x72,0x00,0x07},
                                   {0x31,0x11,0x11,0x20,0x00,0x00,0x77,0x77,0x20,0x77,0x77,0x20,0x77,0x73,0x00,0x77,0x72,0x07,0x77,0x72,0x07,0x77,0x30,0x07,0x77,0x20,0x77,0x77,0x20,0x77},
                                   {0x73,0x11,0x17,0x30,0x00,0x01,0x20,0x77,0x20,0x20,0x73,0x00,0x20,0x10,0x07,0x72,0x00,0x01,0x30,0x07,0x20,0x20,0x77,0x20,0x20,0x11,0x00,0x72,0x00,0x07},
                                   {0x73,0x11,0x17,0x30,0x00,0x01,0x20,0x77,0x20,0x20,0x77,0x20,0x20,0x11,0x00,0x72,0x00,0x00,0x77,0x77,0x20,0x77,0x77,0x20,0x73,0x11,0x00,0x72,0x00,0x07}};
      };
    }




    template <>
    struct WatchFaceTraits<WatchFace::Maze> {
      static constexpr WatchFace watchFace = WatchFace::Maze;
      static constexpr const char* name = "Maze";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceMaze(controllers.lvgl,
                                          controllers.dateTimeController,
                                          controllers.settingsController/*,
                                          controllers.batteryController,
                                          controllers.bleController*/);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}