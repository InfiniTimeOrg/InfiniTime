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
      // reading up on bitfields and their potential inconsistencies scared me away from them...
      struct MazeTile {
        static constexpr uint8_t UPMASK = 0b0001;
        static constexpr uint8_t LEFTMASK = 0b0010;
        static constexpr uint8_t FLAGEMPTYMASK = 0b0100;
        static constexpr uint8_t FLAGGENMASK = 0b1000;
        uint8_t map = 0;

        // Set flags on given tile. Returns the object so they can be chained.
        // Am I cool for aligning it like this? Or does it just make it hard to read...
        MazeTile setUp(bool value)        {map = (map&~UPMASK)        | (value * UPMASK);        return *this;}
        MazeTile setLeft(bool value)      {map = (map&~LEFTMASK)      | (value * LEFTMASK);      return *this;}
        MazeTile setFlagEmpty(bool value) {map = (map&~FLAGEMPTYMASK) | (value * FLAGEMPTYMASK); return *this;}
        MazeTile setFlagGen(bool value)   {map = (map&~FLAGGENMASK)   | (value * FLAGGENMASK);   return *this;}

        // Get flags on given tile
        bool getUp()        {return map & UPMASK;}
        bool getLeft()      {return map & LEFTMASK;}
        bool getFlagEmpty() {return map & FLAGEMPTYMASK;}
        bool getFlagGen()   {return map & FLAGGENMASK;}
      };




      // custom PRNG for the maze to easily allow it to be deterministic for any given minute
      class MazeRNG {
      public:
        MazeRNG(uint64_t start_seed = 64) {seed(start_seed);}

        // Reseed the generator. Handles any input well. If seed is 0, acts as though it was seeded with 1.
        void seed(uint64_t seed) {state = seed ? seed : 1; rand();}

        // RNG lifted straight from https://en.wikipedia.org/wiki/Xorshift#xorshift* (asterisk is part of the link)
        uint32_t rand() {
          state ^= state >> 12;
          state ^= state << 25;
          state ^= state >> 27;
          return state * 0x2545F4914F6CDD1DULL >> 32;
        }

        // Random in range, inclusive on both ends (don't make max<min)
        uint32_t rand(uint32_t min, uint32_t max) {assert(max>=min); return (rand()%(max-min+1))+min;}

      private:
        uint64_t state;
      };




      // little bit of convenience for working with directions
      enum TileAttr {up, down, left, right, flagempty, flaggen};

      // could also be called Field or something. Does not handle stuff like generation or printing,
      // ONLY handles interacting with the board.
      class Maze {
      public:
        Maze();

        // Get and set can work with either xy or indexes
        MazeTile get(int x, int y);
        MazeTile get(int index);

        void set(int x, int y, MazeTile tile);
        void set(int index, MazeTile tile);

        // fill() fills all tiles in the maze with a given value, optionally with a mask on what bits to change.
        // Only cares about the low 4 bits in the value and mask.
        // (use the MazeTile::*MASK values)
        void fill(MazeTile tile, uint8_t mask = 0xFF);
        void fill(uint8_t value, uint8_t mask = 0xFF);

        // Allows abstractly setting a given side on a tile. Supports down and right for convenience.
        void setSide(int x, int y, TileAttr attr, bool value);
        void setSide(int index, TileAttr attr, bool value);

        // Same as setSide, just getting.
        bool getSide(int x, int y, TileAttr attr);
        bool getSide(int index, TileAttr attr);

        // Paste onto an empty board. Marks a tile as not empty if any neighboring walls are gone.
        // toPaste is a 1d array of uint8_t, only containing the two wall bits (left then up). So that's 4 walls in a byte.
        // If you have a weird number of tiles (like 17), just pad it out to the nearest byte. The function ignores any extra data.
        // Always places values left to right, top to bottom. Coords are inclusive.
        void pasteMazeSeed(int x1, int y1, int x2, int y2, const uint8_t toPaste[]);

        // 10x10 px tiles on the maze = 24x24 (on 240px screen)
        static constexpr int WIDTH = 24;
        static constexpr int HEIGHT = 24;
        static constexpr int TILESIZE = 10;
        // the actual size of the entire map. only store 4 bits per block, so 2 blocks per byte
        static constexpr int FLATSIZE = WIDTH*HEIGHT/2;

      private:
        // the internal map. Doesn't actually store MazeTiles, but packs their contents to 2 tiles per byte.
        uint8_t mazemap[FLATSIZE];
      };



      class ConfettiParticle {
      public:
        // steps the confetti simulation. Importantly, updates the maze equivalent position.
        // Returns true if the particle has finished processing, else false.
        // Need to save the particle position elsewhere before stepping to be able to clear the old particle position (if redraw needed).
        // I couldn't really figure a better way to do it other than saving the old positions in the class itself and that's just awful.
        bool step();

        // reset position and generate new velocity using the passed prng
        void reset(MazeRNG &prng);

        // x and y positions of the particle. Positions are in pixels, so just divide by tile size to get the tile it's in.
        int16_t tileX() const {return xpos > 0 ? xpos/((float)Maze::TILESIZE) : -1;}  // need positive check else particles can get stuck to left wall
        int16_t tileY() const {return ypos/((float)Maze::TILESIZE);}

        // color of the particle
        lv_color_t color;

      private:
        // positions and velocities of particle, in pixels and pixels/step (~50 steps per second)
        float xpos;
        float ypos;
        float xvel;
        float yvel;

        // first apply gravity, then apply damping factor, then add velocity to position
        static constexpr float GRAVITY = 0.3;  // added to yvel every step (remember up is -y)
        static constexpr float DAMPING_FACTOR = 0.99;  // keep this much of the velocity every step (applied after gravity)
      };



      // What is currently being displayed.
      // Watchface is normal operation; anything else is an easter egg. Really only used to indicate what
      //  should be displayed when the screen refreshes.
      enum Displaying {watchface, blank, loss, amogus, autismcreature, foxgame, reminder, pinetime};

      // The watchface itself
      class WatchFaceMaze : public Screen {
      public:
        WatchFaceMaze(Pinetime::Components::LittleVgl&,
                      Controllers::DateTime&,
                      Controllers::Settings&,
                      Controllers::MotorController&,
                      const Controllers::Battery&,
                      const Controllers::Ble&);
        ~WatchFaceMaze() override;

        // Functions required for app operation.
        void Refresh() override;
        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

      private:
        // Functions related to touching the screen, for better separation of processes
        bool HandleLongTap();
        bool HandleTap();
        bool HandleSwipe(uint8_t direction);

        // Seed the maze with whatever the currentState dictates should be shown
        void SeedMaze();

        // Put time and date onto the screen. Acts as a seed to generate from.
        void PutTimeDate();

        // Very simple function which just resets the maze to what is considered blank
        void InitializeMaze();

        // Generate the maze around whatever the maze was seeded with.
        // MAZE MUST BE SEEDED ELSE ALL YOU'LL GENERATE IS AN INFINITE LOOP!
        // If seed has disconnected components, maze will not be perfect.
        void GenerateMaze();

        // Generates a single path starting at the provided x,y coords
        void GeneratePath(int x, int y);

        // If the maze has any disconnected components (such as if seeded with multiple disconnected blocks),
        // poke holes to force all components to be connected.
        void ForceValidMaze();

        // Draws the maze to the screen.
        void DrawMaze();

        // Fill in the inside of a maze square. Wall states don't affect this; it never draws in the area where walls go.
        // Generic, but only actually used for confetti.
        void DrawTileInner(int16_t x, int16_t y, lv_color_t color);

        // Draw the indicators at the top right.
        void UpdateBatteryDisplay(bool forceRedraw = false);
        void UpdateBleDisplay(bool forceRedraw = false);
        void ClearIndicators();

        // Draw and generally deal with confetti
        void ProcessConfetti();
        void ClearConfetti();


        // Stuff necessary for interacting with the OS and whatnot
        lv_task_t* taskRefresh;
        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;
        Controllers::MotorController& motor;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Components::LittleVgl& lvgl;

        // Maze and internal RNG (so it doesn't mess with other things by reseeding the regular C RNG provider)
        Maze maze;
        MazeRNG prng;

        // Time stuff.
        // currentDateTime is used for keeping track of minutes. It's what refreshes the screen every minute.
        // realTime is present due to what seems to be a race condition if dateTimeController.CurrentDateTime() is called
        //  by two things at once. It gets updated every refresh, or roughly every 20ms. Do not use it for anything high precision.
        //  (I really don't understand, but doing this decreased infinisim crashes due to mutexes releasing without being held so whatever)
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> realTime {};

        // Indicator stuff
        Utility::DirtyValue<uint8_t> batteryPercent;
        Utility::DirtyValue<bool> charging;
        Utility::DirtyValue<bool> bleConnected;

        // Confetti for autism creature
        // Infinisim warning: because each confetti moving causes 2 draw calls, this is really slow in Infinisim. Lower if using Infinisim.
        constexpr static uint16_t CONFETTI_COUNT = 1;
        ConfettiParticle confettiArr[CONFETTI_COUNT];  // can freely increase/decrease number of particles
        bool initConfetti = false;  // don't want to touch confettiArr in touch event handler, so use a flag and do it in refresh()
        bool confettiActive = false;

        // Buffers for use during printing. There's two it flips between because if there was only one,
        //  it would start being overwritten before the DMA finishes, so it'd corrupt parts of the display.
        // activeBuffer is, well, the currently active one. Flip with `activeBuffer = (activeBuffer==buf1) ? buf2 : buf1;`
        lv_color_t buf1[480];
        lv_color_t buf2[480];
        lv_color_t *activeBuffer = buf1;

        // All concerning the printing of the screen. If screenRefreshRequired is set and screenRefreshTargetTime is
        //  greater than the current time, it waits until that target time before refreshing. Otherwise, it refreshes
        //  immediately when it sees that screenRefreshRequired is set.
        // pausedGeneration is used if the maze took too long to generate, so it lets other processes get cpu time.
        //  It really should never trigger with this small 24x24 maze.
        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> screenRefreshTargetTime {};
        bool screenRefreshRequired = false;
        bool pausedGeneration = false;

        // Number data and AM/PM data for displaying time
        constexpr static uint8_t numbers[10][15] /*6x10*/ = {
                              {0xF5,0x7C,0x01,0x8F,0x88,0xF8,0x8F,0x88,0xF8,0x8F,0x88,0xF8,0x85,0x0E,0x03},
                              {0xF5,0xFC,0x0F,0x80,0xFF,0x8F,0xF8,0xFF,0x8F,0xF8,0xFF,0x8F,0xD0,0x58,0x00},
                              {0xF5,0x7C,0x01,0x8F,0x88,0xF8,0xFF,0x0F,0xC0,0xF0,0x3C,0x0F,0x80,0x58,0x00},
                              {0xF5,0x7C,0x01,0x8F,0x8F,0xF8,0xFD,0x0F,0x80,0xFF,0x8D,0xF8,0x85,0x0E,0x03},
                              {0xDF,0xD8,0xF8,0x8F,0x88,0xF8,0x85,0x08,0x00,0xFF,0x8F,0xF8,0xFF,0x8F,0xF8},
                              {0xD5,0x58,0x00,0x8F,0xF8,0xFF,0x85,0x78,0x01,0xFF,0x8F,0xF8,0xD5,0x08,0x03},
                              {0xF5,0x7C,0x03,0x8F,0xF8,0xFF,0x85,0x78,0x01,0x8F,0x88,0xF8,0x85,0x0E,0x03},
                              {0xD5,0x58,0x00,0xFF,0x8F,0xF8,0xFF,0x0F,0xE3,0xFE,0x3F,0xC3,0xF8,0xFF,0x8F},
                              {0xF5,0x7C,0x01,0x8F,0x88,0xF0,0x84,0x3E,0x01,0xC3,0x88,0xF8,0x85,0x0E,0x03},
                              {0xF5,0x7C,0x01,0x8F,0x88,0xF8,0x85,0x0E,0x00,0xFF,0x8F,0xF8,0xF5,0x0E,0x03}};
        constexpr static uint8_t am[10] /*5x8*/ = {0xF5,0xF0,0x18,0xE2,0x38,0x84,0x20,0x08,0xE2,0x38};
        constexpr static uint8_t pm[10] /*5x8*/ = {0xD5,0xE0,0x18,0xE2,0x38,0x84,0x20,0x38,0xFE,0x3F};
        constexpr static uint8_t blankseed[1] /*4x1*/ = {0xD5};
        constexpr static uint8_t indicatorSpace[3] /*3x3*/ = {0xF6,0x8A,0x00};

        // Used for swipe sequences for easter eggs.
        // currentState is what is being displayed. It's generally categorized into "watchface" and "not watchface".
        // lastInputTime is for long clicking on the main watchface. If you long click twice in 2 seconds, it goes to the secret input screen.
        // currentCode is the current swipe sequence that's being worked on
        Displaying currentState = Displaying::watchface;
        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> lastInputTime {};
        uint8_t currentCode[8];

        // Input codes for secret swipe gestures
        // Note that the codes are effectively backwards; the currentCode is a stack being pushed from the left. Values are 0-3, clockwise from up.
        constexpr static uint8_t lossCode[8] = {0,0,2,2,3,1,3,1};  // RLRLDDUU (konami code backwards)
        constexpr static uint8_t amogusCode[8] = {1,3,1,3,2,2,0,0};  // UUDDLRLR (konami code)
        constexpr static uint8_t autismCode[8] = {3,1,3,1,3,1,3,1};  // RLRLRLRL (pet pet pet pet)
        constexpr static uint8_t foxCode[7] = {0,1,0,3,2,1,2};  // the first of a type of secret in a curious game :3
        constexpr static uint8_t reminderCode[4] = {3,2,1,0};  // URDL
        constexpr static uint8_t pinetimeCode[8] = {1,2,3,0,1,2,3,0};  // ULDRULDR (two counterclockwise rotations)

        // Maze data for secrets. These are pasted onto the screen when the corresponding code is entered.
        constexpr static uint8_t loss[105] /*21x20*/ = {0xFD,0xFF,0xFF,0xF7,0xFF,0xFE,0x3F,0xFF,0xF8,0xFF,0xFF,0x8F,0xFF,0xFE,0x3F,0xDF,0xE3,0xFF,0xFF,0x8F,0xE3,0xF8,0xFF,0xFF,0xE3,0xF8,0xFE,0x3F,0xFF,0xF8,0xFE,0x3F,0x8F,0xFF,0xFE,0x3F,0x8F,0xE3,0xFF,0xFF,0x8F,0xE3,0xF8,0xFF,0xFF,0xE3,0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0xDF,0xFF,0x7F,0xFF,0x8F,0xE3,0xFF,0x8F,0xFF,0xE3,0xF8,0xFF,0xE3,0xFF,0xF8,0xFE,0x3F,0xF8,0xFF,0xFE,0x3F,0x8F,0xFE,0x3F,0xFF,0x8F,0xE3,0xFF,0x8F,0xFF,0xE3,0xF8,0xFF,0xE3,0xFF,0xF8,0xFE,0x3F,0xF8,0xF5,0x56,0x3F,0x8F,0xFE,0x38,0x00};
        constexpr static uint8_t amogus[114] /*19x24*/ = {0xFF,0xFF,0x55,0x7F,0xFF,0xFF,0xD0,0x00,0x7F,0xFF,0xFE,0x0F,0xF8,0x7F,0xFF,0xF0,0xFF,0x40,0x7F,0xFF,0x83,0xF0,0x00,0x5F,0xFE,0x3F,0x0F,0xFE,0x1F,0x50,0xF8,0xFF,0xFE,0x30,0x03,0xE3,0xFF,0xF8,0x8F,0x8F,0x87,0xFF,0xC2,0x3E,0x3F,0x85,0x54,0x38,0xF8,0xFF,0xE0,0x03,0xE3,0xE3,0xFF,0xFF,0x8F,0x8F,0x8F,0xFF,0xFE,0x3E,0x3E,0x3F,0xFF,0xF8,0xF8,0xF8,0xFF,0xFF,0xE3,0xE3,0xE3,0xFF,0xFF,0x8F,0x8F,0x8F,0xFF,0xFE,0x3E,0x14,0x3F,0xD7,0xF8,0xFE,0x00,0xFC,0x07,0xE3,0xFF,0x83,0xE3,0x8F,0x8F,0xFF,0x8F,0x8E,0x3E,0x3F,0xFE,0x3E,0x38,0xF8,0xFF,0xF8,0x50,0xE1,0x43,0xFF,0xE0,0x03,0x80,0x3F};
        constexpr static uint8_t autismcreature[126] /*24x21*/ = {0xFD,0x55,0x55,0x7F,0xFF,0xFF,0xF0,0x00,0x00,0x17,0xFF,0xFF,0xC3,0xFF,0xFF,0x81,0xFF,0xFF,0x8F,0xFF,0xFF,0xF8,0x7F,0xFF,0xBF,0x5F,0xF5,0xFE,0x3F,0xFF,0xBF,0x87,0xF8,0x7E,0x3F,0xFF,0xB9,0x03,0x90,0x3E,0x3F,0xFF,0xB8,0x03,0x80,0x3E,0x3F,0xFF,0xBE,0x0F,0xE0,0xFE,0x15,0x57,0x9F,0xFF,0xFF,0xFC,0x00,0x01,0x87,0xFF,0xFF,0xF0,0x3F,0xF8,0xE1,0x5F,0xFF,0x43,0xFF,0xF8,0xF8,0x05,0x54,0x0F,0xFF,0xF8,0xFE,0x00,0x00,0xFF,0xFF,0xF8,0xFE,0x3F,0xFF,0xFF,0xFF,0xF8,0xFE,0x3F,0x7F,0xD7,0xFD,0xF8,0xFE,0x3E,0x3F,0x01,0xF8,0xF8,0xFE,0x3E,0x3E,0x00,0xF8,0xF8,0xFE,0x3E,0x3E,0x38,0xF8,0xF8,0xFE,0x14,0x14,0x38,0x50,0x50,0xFF,0x80,0x00,0xFE,0x00,0x03};
        constexpr static uint8_t foxgame[132] /*24x22*/ = {0xFF,0xD7,0xFF,0xFF,0xF5,0xFF,0xFD,0x01,0x7F,0xFF,0x40,0x5F,0xF0,0x38,0x1F,0xFC,0x0E,0x07,0xC3,0xFF,0x87,0xF0,0xFF,0xE1,0x8F,0xFF,0xE3,0xE3,0xFF,0xF8,0x8F,0xFF,0xFF,0xE1,0xFF,0xF0,0x8F,0xFF,0xFF,0xE0,0x5F,0x43,0x8F,0xFF,0xFF,0xE0,0x04,0x0F,0x8F,0xFF,0xFF,0xE3,0xE0,0xFF,0x8F,0xFF,0xFF,0xE3,0xFF,0xFF,0x85,0x55,0x55,0x41,0x55,0x55,0x80,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xDF,0xFF,0xFF,0xF7,0xFF,0xFF,0x8F,0xFF,0xFF,0xE3,0xFF,0xFF,0x8F,0xFF,0xFF,0xE3,0xFF,0xFF,0x8F,0xFF,0xFF,0xE3,0xFF,0xFF,0x8F,0xFF,0xFF,0xE3,0xFF,0xFF,0x87,0xFF,0xFF,0xE1,0xFF,0xFF,0xE1,0x7F,0xFF,0xF8,0x5F,0xFF,0xF8,0x17,0xFF,0xFE,0x05,0xFF,0xFF,0x83,0xFF,0xFF,0xE0,0xFF};
        constexpr static uint8_t reminder[102] /*24x17*/ = {0xFF,0xD5,0xF7,0xDF,0x57,0xFF,0xFF,0x80,0xE3,0x8E,0x03,0xFF,0xFF,0xE3,0xE3,0x8E,0x3F,0xFF,0xFF,0xE3,0xE1,0x0E,0x17,0xFF,0xFF,0xE3,0xE0,0x0E,0x03,0xFF,0xFF,0xE3,0xE3,0x8E,0x3F,0xFF,0xFF,0xE3,0xE3,0x8E,0x17,0xFF,0xFF,0xE3,0xE3,0x8E,0x03,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF5,0x7F,0x5F,0xF5,0x5F,0xD5,0xC0,0x3C,0x07,0xC0,0x07,0x80,0x8F,0xF8,0xE3,0x88,0xE3,0x8F,0x8F,0xF8,0xE3,0x88,0xE3,0x85,0x8F,0x78,0x43,0x88,0xE3,0x80,0x8E,0x38,0x03,0x8F,0xE3,0x8F,0x84,0x38,0xE3,0x8F,0xE3,0x85,0xE0,0xF8,0xE3,0x8F,0xE3,0x80};
        //constexpr static uint8_t foxface[144] /*24x24*/ = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFD,0x7F,0xFF,0xFF,0xFF,0xFF,0xF8,0x17,0xFF,0xFF,0xFF,0xFF,0xF8,0x01,0x7F,0xFF,0xF5,0x5F,0xF8,0xF8,0x1F,0xFF,0x40,0x07,0xF8,0xFF,0x8F,0xF4,0x0F,0xE3,0xF8,0x7F,0x85,0x40,0xFF,0xC3,0xF8,0x1F,0xE0,0x0F,0xFD,0x03,0xF8,0xE7,0xFF,0xFF,0xF3,0xE3,0xF0,0xFF,0xFF,0xFF,0xFF,0xE3,0xC0,0xFD,0x7F,0xFF,0xFF,0xCF,0x8F,0xFE,0x1F,0xFD,0x7F,0x8F,0xBF,0xE4,0x0F,0xFE,0x1F,0x87,0xFF,0xE0,0x0F,0xE4,0x0F,0xE1,0xFF,0xF8,0x3F,0xE0,0x0F,0xF8,0xDF,0xFF,0xFF,0xF8,0x3F,0xF8,0xE5,0x7F,0xF5,0xFF,0xFF,0xF8,0xFF,0x95,0x40,0x7F,0xFF,0xFE,0xFF,0xFF,0xE0,0x15,0x55,0x54,0xBF,0xFF,0xF8,0xFF,0xFF,0xFF,0x9F,0xFF,0xFF,0xFF,0xFF,0xFD,0x87,0xFF,0xFF,0xFF,0xFF,0xF0,0xE1,0x5F,0xFF,0xFF,0xFF,0x43,0xF8,0x05,0x5F,0xFF,0xD4,0x3F};
        constexpr static uint8_t pinetime[120] /*20x24*/ = {0xFF,0xFF,0xF7,0xFF,0xFF,0xFF,0xFF,0xC1,0xFF,0xFF,0xFF,0xFF,0x00,0x7F,0xFF,0xFF,0xF6,0x00,0x37,0xFF,0xFF,0xC1,0x63,0x41,0xFF,0xFF,0x80,0xD5,0x80,0xFF,0xFF,0xB5,0x00,0x56,0xFF,0xF7,0xC0,0x00,0x01,0xF7,0xE1,0x60,0x00,0x03,0x43,0xE0,0x15,0x80,0xD4,0x03,0xE0,0x00,0x5D,0x00,0x03,0xE0,0x00,0xD5,0x80,0x03,0xE0,0x35,0x00,0x56,0x03,0xE3,0x40,0x00,0x01,0x63,0x96,0x00,0x00,0x00,0x34,0x81,0x60,0x00,0x03,0x40,0xE0,0x15,0x80,0xD4,0x03,0xE0,0x00,0x77,0x00,0x03,0xF8,0x00,0xC1,0x80,0x0F,0xF8,0x0D,0x00,0x58,0x0F,0xFF,0xD0,0x00,0x05,0xFF,0xFF,0xE0,0x00,0x03,0xFF,0xFF,0xFE,0x00,0x3F,0xFF,0xFF,0xFF,0xE3,0xFF,0xFF};\
      };
    }




    template <>
    struct WatchFaceTraits<WatchFace::Maze> {
      static constexpr WatchFace watchFace = WatchFace::Maze;
      static constexpr const char* name = "Maze";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceMaze(controllers.lvgl,
                                          controllers.dateTimeController,
                                          controllers.settingsController,
                                          controllers.motorController,
                                          controllers.batteryController,
                                          controllers.bleController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}