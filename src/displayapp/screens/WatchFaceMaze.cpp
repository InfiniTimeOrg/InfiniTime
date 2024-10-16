#include <sys/unistd.h>
#include "displayapp/screens/WatchFaceMaze.h"

#include "Tile.h"

using namespace Pinetime::Applications::Screens;

// Despite being called Maze, this really is only a relatively simple wrapper for the specialized
// (fake) 2d array on which the maze structure is built. It should only have manipulations for
// the structure, generating and printing should be handled elsewhere.
Maze::Maze() {
  std::fill_n(mazemap, FLATSIZE, 0);
}

// only returns 4 bits (since that's all that's stored)
// returns 0 in case of out of bounds access
MazeTile Maze::get(int x, int y) {
  if (x<0||x>WIDTH||y<0||y>HEIGHT) {return MazeTile(0b0011);}
  return get((y * WIDTH) + x);
}
MazeTile Maze::get(int index) {
  if (index < 0 || index/2 >= FLATSIZE) {return MazeTile(0b0011);}
  // odd means right (low) nibble, even means left (high) nibble
  if (index & 0b1) return MazeTile(mazemap[index/2] & 0b00001111);
  else             return MazeTile(mazemap[index/2] >> 4);
}

// only stores the low 4 bits of the value
// if out of bounds, does nothing
void Maze::set(int x, int y, MazeTile tile) {
  if (x<0||x>WIDTH||y<0||y>HEIGHT) {return;}
  set(y * WIDTH + x, tile);
}
void Maze::set(int index, MazeTile tile) {
  if (index < 0 || index/2 >= FLATSIZE) {return;}
  // odd means right (low) nibble, even means left (high) nibble
  if (index & 0b1) mazemap[index/2] = (mazemap[index/2] & 0b11110000) | tile.map;
  else             mazemap[index/2] = (mazemap[index/2] & 0b00001111) | tile.map << 4;
}

// only operates on the low 4 bits of the uint8_t.
// only sets the bits from the value that are also on in the mask, rest are left alone
// e.g. existing = 1010, value = 0001, mask = 0011, then result = 1001
// (mask defaults to 0xFF which keeps all bits)
void Maze::fill(uint8_t value, uint8_t mask) {
  value = value & 0b00001111;
  value |= value << 4;
  if (mask == 0xFF) {
    // did not include a mask
    std::fill_n(mazemap, FLATSIZE, value);
  } else {
    // included a mask
    mask = mask & 0b00001111;
    mask |= mask << 4;
    value = value & mask; // preprocess mask for value
    mask = ~mask;         // this mask will be applied to the value
    for (uint8_t& mapitem : mazemap) {
      mapitem = (mapitem & mask) + value;
    }
  }
}
inline void Maze::fill(MazeTile tile, uint8_t mask) {
  fill(tile.map, mask);
}

// For quickly manipulating. Also allows better abstraction by allowing setting of down and right sides.
// Silently does nothing if given invalid values.
void Maze::setSide(int index, TileAttr attr, bool value) {
  switch(attr) {
    case up:    set(index, get(index).setUp(value)); break;
    case down:  set(index+WIDTH, get(index+WIDTH).setUp(value)); break;
    case left:  set(index, get(index).setLeft(value)); break;
    case right: set(index+1, get(index+1).setLeft(value)); break;
    case flagempty: set(index, get(index).setFlagEmpty(value)); break;
    case flaggen: set(index, get(index).setFlagGen(value)); break;
  }
}
void Maze::setSide(int x, int y, TileAttr attr, bool value) {
  setSide(y*WIDTH+x, attr, value);
}
bool Maze::getSide(int index, TileAttr attr) {
  switch(attr) {
    case up:    return get(index).getUp();
    case down:  return get(index+WIDTH).getUp();
    case left:  return get(index).getLeft();
    case right: return get(index+1).getLeft();
    case flagempty: return get(index).getFlagEmpty();
    case flaggen: return get(index).getFlagGen();
  }
  return false;
}
bool Maze::getSide(int x, int y, TileAttr attr) {
  return getSide(y*WIDTH+x, attr);
}

void Maze::transparentPaste(int x1, int y1, int x2, int y2, const uint8_t toPaste[]) {
  int pastewidth = x2-x1+1;
  for (int y = 0; y <= y2-y1; y++) {
    for (int x = 0; x <= x2-x1; x++) {
      const int pasteFlatEquiv = y*pastewidth+x;
      const int mazeFlatEquiv = ((y+y1)*(WIDTH/2))+(x+x1);
      // high nibble
      if (!(toPaste[pasteFlatEquiv] & (MazeTile::FLAGEMPTYMASK << 4)))  // only do anything if empty flag is not set
        {mazemap[mazeFlatEquiv] = (mazemap[mazeFlatEquiv] & 0b00001111) | (toPaste[pasteFlatEquiv] & 0b11110000);}
      // low nibble
      if (!(toPaste[pasteFlatEquiv] & MazeTile::FLAGEMPTYMASK))  // only do anything if empty flag is not set
        {mazemap[mazeFlatEquiv] = (mazemap[mazeFlatEquiv] & 0b11110000) | (toPaste[pasteFlatEquiv] & 0b00001111);}
    }
  }
}




WatchFaceMaze::WatchFaceMaze(Pinetime::Components::LittleVgl& lvgl,
                             Controllers::DateTime& dateTimeController,
                             Controllers::Settings& settingsController)
  : dateTimeController {dateTimeController},
    settingsController {settingsController},
    lvgl {lvgl},
    maze {Maze()},
    prng {MazeRNG()} {

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  //Refresh();
}

WatchFaceMaze::~WatchFaceMaze() {
  lv_obj_clean(lv_scr_act());
  lv_task_del(taskRefresh);
}

void WatchFaceMaze::Refresh() {
  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());
  // refresh time if either a minute has passed, or maze is still WIP
  if (pausedGeneration || currentDateTime.IsUpdated()) {
    // if generation wasn't paused (i.e. doing a ground up maze gen), set everything up
    if (!pausedGeneration) {
      prng.seed(currentDateTime.Get().time_since_epoch().count());
      InitializeMaze();
      PutNumbers();  // also works as the seed to generate the maze onto
    }
    GenerateMaze();
    // only draw once maze is fully generated (not paused)
    if (!pausedGeneration) {
      DrawMaze();
    }
  }
}


void WatchFaceMaze::InitializeMaze() {
  // prepare maze by filling it with all walls and empty tiles
  maze.fill(MazeTile().setLeft(true).setUp(true).setFlagEmpty(true));
}

void WatchFaceMaze::PutNumbers() {
  uint8_t hours = dateTimeController.Hours();
  uint8_t minutes = dateTimeController.Minutes();
  // TODO 12 hour format support
  /*
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    // 12 hour format
    // don't have handling, just make a tiny seed and gen a normal maze
    maze.setSide(prng.rand(0,Maze::WIDTH), prng.rand(0,Maze::HEIGHT), TileAttr::flagempty, false);
  }
   */
  //maze.setSide(prng.rand(0,Maze::WIDTH), prng.rand(0,Maze::HEIGHT), TileAttr::flagempty, false);

  // 24 hour format only right now
  // top left: hours major digit
  maze.transparentPaste(2, 1, 4, 10, numbers[hours/10]);
  // top right: hours minor digit
  maze.transparentPaste(5, 1, 7, 10, numbers[hours%10]);
  // bottom left: minutes major digit
  maze.transparentPaste(2, 13, 4, 22, numbers[minutes/10]);
  // bottom right: minutes minor digit
  maze.transparentPaste(5, 13, 7, 22, numbers[minutes%10]);
  // put horizontal line connecting the digits horizontally. working in actual tile coords here
  for (int stripLoc : {prng.rand(1,10), prng.rand(13,22)} ) {
    for (int y = stripLoc, x = 9; x > 4; x--) {
      if (maze.getSide(x, y, TileAttr::flagempty) == false) {
        do {
          maze.setSide(x, y, TileAttr::flagempty, false);
          maze.setSide(x, y, TileAttr::right, false);
          x++;
        } while (maze.getSide(x, y, TileAttr::flagempty) == true);
        break; // get out of the for loop
      }
    }
  }
  // now make a vertical stripe in the same way
  for (int x = prng.rand(4,15), y = 10; y > 0; y--) {
    if (maze.getSide(x, y, TileAttr::flagempty) == false) {
      do {
        maze.setSide(x, y, TileAttr::flagempty, false);
        maze.setSide(x, y, TileAttr::down, false);
        y++;
      } while (maze.getSide(x, y, TileAttr::flagempty) == true);
      break; // get out of the for loop
    }
  }
}

void WatchFaceMaze::GenerateMaze() {
  int x, y, oldx, oldy;
  // task should only run for 3/4 the time it takes for the task to refresh.
  // Will go over; only checks once it's finished with current line. It won't go too far over though.
  auto maxGenTarget = dateTimeController.CurrentDateTime() + std::chrono::milliseconds((taskRefresh->period*3)/4);

  while (true) {
    // FIND POSITION TO START BRANCH FROM
    for (uint8_t i = 0; i < 30; i++) {
      x = prng.rand(0, Maze::WIDTH-1);
      y = prng.rand(0, Maze::HEIGHT-1);
      if (maze.getSide(x,y, TileAttr::flagempty)) {break;}  // found solution tile
      if (i == 29) {
        // failed all 30 attempts (this is inside the for loop for 'organization')
        // find solution tile slowly but guaranteed
        int count = 0;
        // count number of valid tiles
        for (int j = 0; j < Maze::WIDTH*Maze::HEIGHT; j++)
        {if (maze.getSide(j, TileAttr::flagempty)) {count++;}}
        if (count == 0) {
          // all tiles filled; maze gen done
          pausedGeneration = false;
          return;
        }
        // if maze gen not done, select random index from valid tiles to start from
        //count = prng.rand(0, count);
        //printf("bruteforce chosen count: %i", count);
        for (int j = 0; j < Maze::WIDTH*Maze::HEIGHT; j++) {
          if (maze.getSide(j, TileAttr::flagempty)) {count--;}
          if (count == 0) {
            y = j / Maze::WIDTH;
            x = j % Maze::WIDTH;
            break;
          }
        }
      }
    }
    // function now has a valid position a maze line can start from in x and y
    oldx = -1, oldy = -1;
    // GENERATE A SINGLE PATH
    uint8_t direction;  // which direction the cursor moved in
    while (true) {
      maze.setSide(x, y, TileAttr::flagempty, false);  // no longer empty
      maze.setSide(x, y, TileAttr::flaggen, true);     // in generation
      oldx = x, oldy = y;
      // move to next tile
      // this is very scuffed, but it prevents backtracking.
      while (true) {
        // effectively turn tbe cursor so it will never go straight back
        switch (direction = prng.rand(0,3)) {
          case 0:  // moved up
            if (y <= 0 || !maze.getSide(x,y,TileAttr::up)) {continue;}
            y -= 1; break;
          case 1:  // moved left
            if (x <= 0 || !maze.getSide(x,y,TileAttr::left)) {continue;}
            x -= 1; break;
          case 2:  // moved down
            if (y >= Maze::HEIGHT-1 || !maze.getSide(x,y,TileAttr::down)) {continue;}
            y += 1; break;
          case 3:  // moved right
            if (x >= Maze::WIDTH-1 || !maze.getSide(x,y,TileAttr::right)) {continue;}
            x += 1; break;
        }
        break;
      }
      // moved to next tile. check if looped in on self
      if (!maze.getSide(x, y, TileAttr::flaggen)) {
        // did NOT loop in on self, simply remove wall and move on
        switch (direction) {
          case 0:  // moved up
            maze.setSide(x,y,TileAttr::down, false); break;
          case 1:  // moved left
            maze.setSide(x,y,TileAttr::right, false); break;
          case 2:  // moved down
            maze.setSide(x,y,TileAttr::up, false); break;
          case 3:  // moved right
            maze.setSide(x,y,TileAttr::left, false); break;
        }
        // if attached to main maze, path finished generating
        if (!maze.getSide(x, y, TileAttr::flagempty)) {break;}
      } else {
        // DID loop in on self, track down and eliminate loop
        // targets are the coordinates of where it needs to backtrack to
        int targetx = x, targety = y;
        x = oldx, y = oldy;
        while (x != targetx || y != targety) {
          if (y > 0 && (maze.getSide(x, y, TileAttr::up) == false)) {  // backtrack up
            maze.setSide(x, y, TileAttr::up, true);
            maze.setSide(x, y, TileAttr::flaggen, false);
            maze.setSide(x, y, TileAttr::flagempty, true);
            y -= 1;
          } else if (x > 0 && (maze.getSide(x, y, TileAttr::left) == false)) {  // backtrack left
            maze.setSide(x, y, TileAttr::left, true);
            maze.setSide(x, y, TileAttr::flaggen, false);
            maze.setSide(x, y, TileAttr::flagempty, true);
            x -= 1;
          } else if (y < Maze::HEIGHT-1 && (maze.getSide(x, y, TileAttr::down) == false)) {  // backtrack down
            maze.setSide(x, y, TileAttr::down, true);
            maze.setSide(x, y, TileAttr::flaggen, false);
            maze.setSide(x, y, TileAttr::flagempty, true);
            y += 1;
          } else if (x < Maze::WIDTH && (maze.getSide(x, y, TileAttr::right) == false)) {  // backtrack right
            maze.setSide(x, y, TileAttr::right, true);
            maze.setSide(x, y, TileAttr::flaggen, false);
            maze.setSide(x, y, TileAttr::flagempty, true);
            x += 1;
          }
        }
      }
    }
    // mark all tiles as finalized and not in generation by removing ALL flaggen's
    maze.fill(0, MazeTile::FLAGGENMASK);
    if (dateTimeController.CurrentDateTime() > maxGenTarget) {
      pausedGeneration = true;
      return;
    }
  }
  // execution never gets here! it returns earlier in the function.
}

void WatchFaceMaze::DrawMaze() {
  // this used to be nice code, but it was retrofitted to print offset by 1 pixel for a fancy border.
  // I'm not proud of the logic but it works.
  lv_area_t area;
  lv_color_t *curbuf = buf1;
  // print horizontal lines
  area.x1 = 1;
  area.x2 = 238;
  for (int y = 1; y < Maze::HEIGHT; y++) {
    for (int x = 0; x < Maze::WIDTH; x++) {
      if (maze.get(x, y).getUp())  {std::fill_n(&curbuf[x*Maze::TILESIZE], Maze::TILESIZE, LV_COLOR_WHITE);}
      else                         {std::fill_n(&curbuf[x*Maze::TILESIZE], Maze::TILESIZE, LV_COLOR_BLACK);}
    }
    std::copy_n(curbuf, 238, &curbuf[238]);
    area.y1 = Maze::TILESIZE * y - 1;
    area.y2 = Maze::TILESIZE * y;
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, curbuf);
    curbuf = (curbuf==buf1) ? buf2 : buf1;  // switch buffer
  }
  // print vertical lines
  area.y1 = 1;
  area.y2 = 238;
  for (int x = 1; x < Maze::WIDTH; x++) {
    for (int y = 0; y < Maze::HEIGHT; y++) {
      MazeTile curblock = maze.get(x,y);
      // handle corners: if any of the touching lines are present, add corner. else leave it black
      if (curblock.getUp() || curblock.getLeft() || maze.get(x-1,y).getUp() || maze.get(x,y-1).getLeft())
            {std::fill_n(&curbuf[y*Maze::TILESIZE*2], 4, LV_COLOR_WHITE);}
      else  {std::fill_n(&curbuf[y*Maze::TILESIZE*2], 4, LV_COLOR_BLACK);}

      if (curblock.getLeft()) {std::fill_n(&curbuf[y*Maze::TILESIZE*2+4], Maze::TILESIZE*2-4, LV_COLOR_WHITE);}
      else                    {std::fill_n(&curbuf[y*Maze::TILESIZE*2+4], Maze::TILESIZE*2-4, LV_COLOR_BLACK);}
    }
    area.x1 = Maze::TILESIZE * x - 1;
    area.x2 = Maze::TILESIZE * x;
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, &curbuf[4]);
    curbuf = (curbuf==buf1) ? buf2 : buf1;  // switch buffer
  }
  // print borders
  std::fill_n(curbuf, 240, LV_COLOR_GRAY);
  for (int i = 0; i < 4; i++) {
    if (i==0)      {area.x1=0;   area.x2=239; area.y1=0;   area.y2=0;  } // top
    else if (i==1) {area.x1=0;   area.x2=239; area.y1=239; area.y2=239;} // bottom
    else if (i==2) {area.x1=0;   area.x2=0;   area.y1=0;   area.y2=239;} // left
    else if (i==3) {area.x1=239; area.x2=239; area.y1=0;   area.y2=239;} // right
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, curbuf);
  }
}