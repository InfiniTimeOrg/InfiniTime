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
// returns walls but unset flags in case of out of bounds access
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
inline void Maze::fill(MazeTile tile, uint8_t mask)
  {fill(tile.map, mask);}


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

// Paste a set of tiles into the given coords.
void Maze::pasteMazeSeed(int x1, int y1, int x2, int y2, const uint8_t toPaste[]) {
  // Assumes a maze with empty flags all true, and all walls present
  uint16_t flatcoord = 0;  // the position in the array (inside the byte, so index 1 would be mask 0b00110000 in the first byte)
  for (int y = y1; y <= y2; y++) {
    for (int x = x1; x <= x2; x++) {
      // working holds the target wall (bit 2 for left wall, bit 1 for up wall)
      uint8_t working = (toPaste[flatcoord/4] & (0b11 << ((3-(flatcoord%4))*2))) >> ((3-(flatcoord%4))*2);

      // handle left wall
      if (!(working & 0b10)) {
        setSide(x, y, TileAttr::left, false);
        setSide(x, y, TileAttr::flagempty, false);
        if (x > 0) setSide(x-1, y, TileAttr::flagempty, false);
      }

      // handle up wall
      if (!(working & 0b01)) {
        setSide(x, y, TileAttr::up, false);
        setSide(x, y, TileAttr::flagempty, false);
        if (y > 0) setSide(x, y-1, TileAttr::flagempty, false);
      }

      flatcoord++;
    }
  }
}




bool ConfettiParticle::step() {
  // first apply gravity (if needed), then dampening, then apply velocity to position.
  xvel *= DAMPING_FACTOR;
  yvel += GRAVITY;
  yvel *= DAMPING_FACTOR;
  xpos += xvel;
  ypos += yvel;

  updateMazeEquiv();

  // return true if particle is finished (went OOB (ignore top; particle can still fall down))
  return (xpos < 0 || xpos > 240 || ypos > 240);
}


void ConfettiParticle::reset(MazeRNG &prng) {
  // always start at bottom middle
  xpos = 120;
  ypos = 240;

  // produces float in range -5 to 5 with resolution of 0.01. very stupid but it works.
  // technically 0.00 has 2x chance of being chosen as other values but idc
  xvel = (((float)prng.rand(0,500))/100);
  if (prng.rand(0,1)) {xvel = -xvel;}
  // float -3 to -8.5 (remember up is -y);
  yvel = -(((float)prng.rand(200,850))/100);

  updateMazeEquiv();
}


// Probably not pixel perfect because of stuff like the maze border but it's close enough for this purpose
// actually it might be but I don't want to think about it more than necessary
void ConfettiParticle::updateMazeEquiv() {
  // calculating tile is easy
  tilex = xpos / 10;
  tiley = ypos / 10;

  // calculating side is gross
  if (tilex%10 > tiley%10) {
    // top or right
    if (tilex%10 > 10-(tiley%10)) {side = 1;}  // right side
    else {side = 0;}  // top side
  } else {
    // bottom or left
    if (tilex%10 > 10-(tiley%10)) {side = 2;}  // bottom side
    else {side = 3;}  // left side
  }

  // and now because I want ONLY unique sides, if it's bottom or right then just change it to the top/left of a neighboring tile
  if (side == 1) {  // right
    tilex++;
    side = 3;
  } else if (side == 2) {  // down
    tiley++;
    side = 0;
  }
}




WatchFaceMaze::WatchFaceMaze(Pinetime::Components::LittleVgl& lvgl,
                             Controllers::DateTime& dateTimeController,
                             Controllers::Settings& settingsController,
                             Controllers::MotorController& motor,
                             const Controllers::Battery& batteryController,
                             const Controllers::Ble& bleController)
  : dateTimeController {dateTimeController},
    settingsController {settingsController},
    motor {motor},
    batteryController {batteryController},
    bleController {bleController},
    lvgl {lvgl},
    maze {Maze()},
    prng {MazeRNG()} {

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  // refreshing here seems to cause issues in infinisim
  //Refresh();
}


WatchFaceMaze::~WatchFaceMaze() {
  lv_obj_clean(lv_scr_act());
  lv_task_del(taskRefresh);
}


void WatchFaceMaze::Refresh() {
  // store time for other functions to use, and update time if needed
  realTime = dateTimeController.CurrentDateTime();
  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(realTime);

  // refresh time if either a minute has passed, or screen refresh timer expired.
  // if minute rolls over while screenrefresh is required, ignore it. the refresh timer will handle it.
  if (pausedGeneration ||  // if generation paused, need to complete it
    (currentState == Displaying::watchface && !screenRefreshRequired && currentDateTime.IsUpdated()) ||  // already on watchface, not waiting for a screen refresh, and time updated
    (screenRefreshRequired && realTime > screenRefreshTargetTime)) {  // waiting on a refresh

    // if generation wasn't paused (i.e. doing a ground up maze gen), set everything up
    if (!pausedGeneration) {
      // only reseed PRNG if got here by the minute rolling over
      if (!screenRefreshRequired) prng.seed(currentDateTime.Get().time_since_epoch().count());
      InitializeMaze();
      SeedMaze();
    }

    // always need to run GenerateMaze().
    GenerateMaze();

    // only draw once maze is fully generated (not paused)
    if (!pausedGeneration) {
      ForceValidMaze();
      if (currentState != Displaying::watchface) {ClearIndicators();}
      DrawMaze();
      screenRefreshRequired = false;
      // if switched to watchface, also add indicators for BLE and battery
      if (currentState == Displaying::watchface) {
        UpdateBatteryDisplay(true);
        UpdateBleDisplay(true);
      }
    }
  }

  // update battery and ble displays if on main watchface
  if (currentState == Displaying::watchface) {
    UpdateBatteryDisplay();
    UpdateBleDisplay();
  }

  // deal with confetti
  // initialize confetti if tapped on autism creature
  if (initConfetti) {
    ClearConfetti();
    for (ConfettiParticle &particle : confettiArr)
      {particle.reset(prng);}
    confettiActive = true;
    initConfetti = false;
  }
  // update confetti if needed
  if (confettiActive) {
    if (currentState != Displaying::autismcreature) {
      // nuke confetti if went to a different display
      ClearConfetti();
      confettiActive = false;
    } else {
      // still on autism creature display, step confetti
      ProcessConfetti();
    }
  }
}

// allow pushing the button to go back to the watchface
bool WatchFaceMaze::OnButtonPushed() {
  if (currentState != Displaying::watchface) {
    screenRefreshRequired = true;
    currentState = Displaying::watchface;
    // reset lastInputTime so it always needs two long taps to get back to blank, even if you're fast
    lastInputTime = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>();
    return true;
  }
  return false;
}

bool WatchFaceMaze::OnTouchEvent(TouchEvents event) {
  // if generation is paused, let it continue working on that. This should really never trigger.
  if (pausedGeneration) {return false;}

  switch (event) {
    case Pinetime::Applications::TouchEvents::LongTap:    return HandleLongTap();
    case Pinetime::Applications::TouchEvents::Tap:        return HandleTap();
    case Pinetime::Applications::TouchEvents::SwipeUp:    return HandleSwipe(0);
    case Pinetime::Applications::TouchEvents::SwipeRight: return HandleSwipe(1);
    case Pinetime::Applications::TouchEvents::SwipeDown:  return HandleSwipe(2);
    case Pinetime::Applications::TouchEvents::SwipeLeft:  return HandleSwipe(3);
    default: return false;  // only handle swipe events
  }
}


bool WatchFaceMaze::HandleLongTap() {
  if (currentState ==  Displaying::watchface) {
    // On watchface; either refresh maze or go to blank state
    if (lastInputTime + std::chrono::milliseconds(2500) > realTime) {
      // long tapped twice in sequence; switch to blank maze
      currentState = Displaying::blank;
      screenRefreshRequired = true;
      std::fill_n(currentCode, sizeof(currentCode), 255);  // clear current code in preparation for code entry
    } else {
      // long tapped not in main watchface; go back to previous state
      screenRefreshRequired = true;
    }
    lastInputTime = realTime;
    motor.RunForDuration(20);
    return true;

  } else {
    // Not on watchface; go back to main watchface
    screenRefreshRequired = true;
    currentState = Displaying::watchface;
    // reset lastInputTime so it always needs two long taps to get back to blank, even if you're fast
    lastInputTime = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>();
    motor.RunForDuration(20);
    return true;
  }
}


bool WatchFaceMaze::HandleTap() {
  // confetti must only display on autismcreature
  if (currentState != Displaying::autismcreature) {return false;}
  // only need to set confettiActive, everything else is handled in functions called by refresh()
  initConfetti = true;
  return true;
}


bool WatchFaceMaze::HandleSwipe(uint8_t direction) {
  // Don't handle any swipes on watchface
  if (currentState == Displaying::watchface) return false;

  // Add the new direction to the swipe list, dropping the last item
  for (int i = sizeof(currentCode)-1; i > 0; i--) {currentCode[i] = currentCode[i-1];}
  currentCode[0] = direction;

  // check if valid code has been entered
  // this structure also has the effect that if code gets entered while on the requested page, it doesn't refresh.
  Displaying newState = currentState;
  if (std::memcmp(currentCode, lossCode, sizeof(lossCode)) == 0)              {newState = Displaying::loss;}   // loss
  else if (std::memcmp(currentCode, amogusCode, sizeof(amogusCode)) == 0)     {newState = Displaying::amogus;}  // amogus
  else if (std::memcmp(currentCode, autismCode, sizeof(autismCode)) == 0)     {newState = Displaying::autismcreature;}  // autismcreature/tbh
  else if (std::memcmp(currentCode, foxCode, sizeof(foxCode)) == 0)           {newState = Displaying::foxgame;}  // foxxo game
  else if (std::memcmp(currentCode, reminderCode, sizeof(reminderCode)) == 0) {newState = Displaying::reminder;}  // reminder
  else if (std::memcmp(currentCode, pinetimeCode, sizeof(pinetimeCode)) == 0) {newState = Displaying::pinetime;}  // pinetime logo

  // only request a screen refresh if state has been updated
  if (newState != currentState) {
    currentState = newState;
    screenRefreshRequired = true;
    motor.RunForDuration(10);
  }
  return true;
}


// Put time and date info on the screen.
void WatchFaceMaze::PutTimeDate() {
  uint8_t hours = dateTimeController.Hours();
  uint8_t minutes = dateTimeController.Minutes();

  // modify hours to account for 12 hour format
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    if (hours == 0) hours = 12;
    if (hours > 12) {
      maze.pasteMazeSeed(18, 15, 22, 22, pm);
      hours -= 12;
    } else {
      maze.pasteMazeSeed(18, 15, 22, 22, am);
    }
  }

  // put time on screen
  maze.pasteMazeSeed(3, 1, 8, 10, numbers[hours / 10]); // top left: hours major digit
  maze.pasteMazeSeed(10, 1, 15, 10, numbers[hours % 10]); // top right: hours minor digit
  maze.pasteMazeSeed(3, 13, 8, 22, numbers[minutes / 10]); // bottom left: minutes major digit
  maze.pasteMazeSeed(10, 13, 15, 22, numbers[minutes % 10]); // bottom right: minutes minor digit

  // reserve some space at the top right to put the battery and BLE indicators there
  maze.pasteMazeSeed(21, 0, 23, 2, indicatorSpace);
}


// seeds the maze with whatever the current state needs
void WatchFaceMaze::SeedMaze() {
  switch (currentState) {
    case Displaying::watchface:
      PutTimeDate(); break;
    case Displaying::blank: { // seed maze with 4 tiles
      const int randx = prng.rand(0, 20);
      const int randy = prng.rand(3, 20);
      maze.pasteMazeSeed(randx, randy, randx + 3, randy, blankseed);
      break;
    }
    case Displaying::loss:
      maze.pasteMazeSeed(2, 2, 22, 21, loss); break;
    case Displaying::amogus:
      maze.pasteMazeSeed(3, 0, 21, 23, amogus); break;
    case Displaying::autismcreature:
      maze.pasteMazeSeed(0, 2, 23, 22, autismcreature); break;
    case Displaying::foxgame:
      maze.pasteMazeSeed(0, 1, 23, 22, foxgame); break;
    case Displaying::reminder:
      maze.pasteMazeSeed(0, 3, 23, 19, reminder); break;
    case Displaying::pinetime:
      maze.pasteMazeSeed(2, 0, 21, 23, pinetime); break;
  }
}


// goes through the maze, finds disconnected segments and connects them
void WatchFaceMaze::ForceValidMaze() {
  // crude maze-optimized flood fill: follow a path until can't move any more, then find some other location to follow from. repeat.
  // this function repurposes flaggen for traversed tiles, so it expects it to be false on all tiles (should be in normal control flow)
  // initialize cursor x and y to bottom right
  int x = Maze::WIDTH-1, y = Maze::HEIGHT - 1;
  while (true) {
    ForceValidMazeLoop:
    maze.setSide(x, y, TileAttr::flaggen, true);
    // move cursor
    if (y > 0 && !maze.getSide(x, y, TileAttr::up) && !maze.getSide(x, y-1, TileAttr::flaggen)) {y--;}
    else if (x < Maze::WIDTH-1 && !maze.getSide(x, y, TileAttr::right) && !maze.getSide(x+1, y, TileAttr::flaggen)) {x++;}
    else if (y < Maze::HEIGHT-1 && !maze.getSide(x, y, TileAttr::down) && !maze.getSide(x, y+1, TileAttr::flaggen)) {y++;}
    else if (x > 0 && !maze.getSide(x, y, TileAttr::left) && !maze.getSide(x-1, y, TileAttr::flaggen)) {x--;}
    else {
      int pokeLocationCount = 0;
      // couldn't find any position to move to, need to set cursor to a different usable location
      for (int proposedy = 0; proposedy < Maze::HEIGHT; proposedy++) {
        for (int proposedx = 0; proposedx < Maze::WIDTH; proposedx++) {
          bool ownState = maze.getSide(proposedx, proposedy, TileAttr::flaggen);

          // if tile to the left is of a different traversal state (is traversed boundary)
          if (proposedx > 0 && (maze.getSide(proposedx-1, proposedy, TileAttr::flaggen) != ownState)) {
            // if found boundary AND can get to it, just continue working from here
            if (maze.getSide(proposedx, proposedy, TileAttr::left) == false) {x = proposedx, y = proposedy; goto ForceValidMazeLoop;}
            pokeLocationCount++;
          }

          // if tile to up is of a different traversal state (is traversed boundary)
          if (proposedy > 0 && (maze.getSide(proposedx, proposedy-1, TileAttr::flaggen) != ownState)) {
            // if found boundary AND can get to it, just continue working from here
            if (maze.getSide(proposedx, proposedy, TileAttr::up) == false) {x = proposedx, y = proposedy; goto ForceValidMazeLoop;}
            pokeLocationCount++;
          }
        }
      }
      // finished scanning maze; there are no locations the cursor can be placed for it to continue scanning

      // if there are no walls that can be poked through to increase reachable area, maze is finished
      if (pokeLocationCount == 0) {return;}

      // if execution gets here, need to poke a hole.
      // choose a random poke location to poke a hole through. pokeLocationCount is now used as an index
      pokeLocationCount = prng.rand(1, pokeLocationCount);
      for (int proposedy = 0; proposedy < Maze::HEIGHT; proposedy++) {
        for (int proposedx = 0; proposedx < Maze::WIDTH; proposedx++) {
          // pretty much a copy of the previous code which FINDS poke locations, but now with the goal of actually doing the poking
          bool ownState = maze.getSide(proposedx, proposedy, TileAttr::flaggen);

          if (proposedx > 0 && (maze.getSide(proposedx-1, proposedy, TileAttr::flaggen) != ownState)) {
            pokeLocationCount--;
            // found the target poke location, poke and loop
            if (pokeLocationCount == 0) {
              maze.setSide(proposedx, proposedy, TileAttr::left, false);
              x = proposedx, y = proposedy;
              goto ForceValidMazeLoop; // continue OUTSIDE loop
            }
          }

          // if tile to up is of a different traversal state (is traversed boundary)
          if (proposedy > 0 && (maze.getSide(proposedx, proposedy-1, TileAttr::flaggen) != ownState)) {
            pokeLocationCount--;
            // found the target poke location, poke and loop
            if (pokeLocationCount == 0) {
              maze.setSide(proposedx, proposedy, TileAttr::up, false);
              x = proposedx, y = proposedy;
              goto ForceValidMazeLoop;  // continue processing
            }
          }
        }
      }
    }
    // done poking a hole in the maze to expand the reachable area
  }
}


// Clear maze
void WatchFaceMaze::InitializeMaze() {
  maze.fill(MazeTile().setLeft(true).setUp(true).setFlagEmpty(true));
}


// Generates the maze around whatever it was seeded with
void WatchFaceMaze::GenerateMaze() {
  int x, y;
  // task should only run for 3/4 the time it takes for the task to refresh.
  // Will go over; only checks once it's finished with current line. It won't go too far over though.
  auto maxGenTarget = dateTimeController.CurrentDateTime() + std::chrono::milliseconds((taskRefresh->period*3)/4);

  while (true) {
    // find position to start generating a path from
    for (uint8_t i = 0; i < 30; i++) {
      x = prng.rand(0, Maze::WIDTH-1);
      y = prng.rand(0, Maze::HEIGHT-1);
      if (maze.getSide(x,y, TileAttr::flagempty)) {break;}  // found solution tile
      if (i == 29) {
        // failed all 30 attempts (this is inside the for loop for 'organization')
        // find solution tile slowly but guaranteed (scan over entire field and choose random valid tile)
        int count = 0;

        // count number of valid tiles
        for (int j = 0; j < Maze::WIDTH*Maze::HEIGHT; j++)
        {if (maze.getSide(j, TileAttr::flagempty)) {count++;}}

        // if no valid tiles are left, maze is done
        if (count == 0) {
          pausedGeneration = false;
          return;
        }

        // if execution gets here then maze gen is not done. select random index from valid tiles to start from
        // 'count' is now used as an index
        count = prng.rand(1, count);
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
    GeneratePath(x, y);

    // if generating paths took too long, suspend it
    if (dateTimeController.CurrentDateTime() > maxGenTarget) {
      pausedGeneration = true;
      return;
    }
  }
  // execution never gets here! it returns earlier in the function.
}


void WatchFaceMaze::GeneratePath(int x, int y) {
  int oldx = -1, oldy = -1;
  uint8_t direction = -1;  // which direction the cursor moved in
  while (true) {
    // set current tile to reflect that it's been worked on
    maze.setSide(x, y, TileAttr::flagempty, false); // no longer empty
    maze.setSide(x, y, TileAttr::flaggen, true);    // in generation
    oldx = x, oldy = y;  // used in backtracking

    // move to next tile
    // the if statements are very scuffed, but they prevent backtracking.
    while (true) {
      switch (direction = prng.rand(0, 3)) {
        case 0: // moved up
          if (y <= 0 || !maze.getSide(x, y, TileAttr::up)) {continue;}
          y -= 1;
          break;
        case 1: // moved left
          if (x <= 0 || !maze.getSide(x, y, TileAttr::left)) {continue;}
          x -= 1;
          break;
        case 2: // moved down
          if (y >= Maze::HEIGHT - 1 || !maze.getSide(x, y, TileAttr::down)) {continue;}
          y += 1;
          break;
        case 3: // moved right
          if (x >= Maze::WIDTH - 1 || !maze.getSide(x, y, TileAttr::right)) {continue;}
          x += 1;
          break;
        default: // invalid
          std::abort();
      }
      break;
    }

    // moved to next tile, check if looped in on self
    if (!maze.getSide(x, y, TileAttr::flaggen)) {
      // did NOT loop in on self, simply remove wall and move on
      switch (direction) {
        case 0: maze.setSide(x, y, TileAttr::down, false); break;  // moved up
        case 1: maze.setSide(x, y, TileAttr::right, false); break;  // moved left
        case 2: maze.setSide(x, y, TileAttr::up, false); break;  // moved down
        case 3: maze.setSide(x, y, TileAttr::left, false); break;  // moved right
      }
      // if attached to main maze, path finished generating
      if (!maze.getSide(x, y, TileAttr::flagempty)) {
        break;
      }

    } else {
      // DID loop in on self, track down and eliminate loop
      // targets are the coordinates of where it needs to backtrack to
      int targetx = x, targety = y;
      x = oldx, y = oldy;
      while (x != targetx || y != targety) {
        if (y > 0 && (maze.getSide(x, y, TileAttr::up) == false)) { // backtrack up
          maze.setSide(x, y, TileAttr::up, true);
          maze.setSide(x, y, TileAttr::flaggen, false);
          maze.setSide(x, y, TileAttr::flagempty, true);
          y -= 1;
        } else if (x > 0 && (maze.getSide(x, y, TileAttr::left) == false)) { // backtrack left
          maze.setSide(x, y, TileAttr::left, true);
          maze.setSide(x, y, TileAttr::flaggen, false);
          maze.setSide(x, y, TileAttr::flagempty, true);
          x -= 1;
        } else if (y < Maze::HEIGHT - 1 && (maze.getSide(x, y, TileAttr::down) == false)) { // backtrack down
          maze.setSide(x, y, TileAttr::down, true);
          maze.setSide(x, y, TileAttr::flaggen, false);
          maze.setSide(x, y, TileAttr::flagempty, true);
          y += 1;
        } else if (x < Maze::WIDTH && (maze.getSide(x, y, TileAttr::right) == false)) { // backtrack right
          maze.setSide(x, y, TileAttr::right, true);
          maze.setSide(x, y, TileAttr::flaggen, false);
          maze.setSide(x, y, TileAttr::flagempty, true);
          x += 1;
        } else {
          // bad backtrack; die
          std::abort();
        }
      }
    }
    // done processing one step, now do it again!
  }
  // finished generating the entire path
  // mark all tiles as finalized and not in generation by removing ALL flaggen's
  maze.fill(0, MazeTile::FLAGGENMASK);
}


void WatchFaceMaze::DrawMaze() {
  // this used to be nice code, but it was retrofitted to print offset by 1 pixel for a fancy border.
  // I'm not proud of the logic but it works.
  lv_area_t area;
  activeBuffer = (activeBuffer==buf1) ? buf2 : buf1;  // switch buffer, who knows if the buffer was used just before this

  // Print horizontal lines
  // This doesn't bother with corners, those just get overwritten by the vertical lines
  area.x1 = 1;
  area.x2 = 238;
  for (int y = 1; y < Maze::HEIGHT; y++) {
    for (int x = 0; x < Maze::WIDTH; x++) {
      if (maze.get(x, y).getUp())  {std::fill_n(&activeBuffer[x*Maze::TILESIZE], Maze::TILESIZE, LV_COLOR_WHITE);}
      else                         {std::fill_n(&activeBuffer[x*Maze::TILESIZE], Maze::TILESIZE, LV_COLOR_BLACK);}
    }
    std::copy_n(activeBuffer, 238, &activeBuffer[238]);
    area.y1 = Maze::TILESIZE * y - 1;
    area.y2 = Maze::TILESIZE * y;
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, activeBuffer);
    activeBuffer = (activeBuffer==buf1) ? buf2 : buf1;  // switch buffer
  }

  // Print vertical lines
  area.y1 = 1;
  area.y2 = 238;
  for (int x = 1; x < Maze::WIDTH; x++) {
    for (int y = 0; y < Maze::HEIGHT; y++) {
      MazeTile curblock = maze.get(x,y);
      // handle corners: if any of the touching lines are present, add corner. else leave it black
      if (curblock.getUp() || curblock.getLeft() || maze.get(x-1,y).getUp() || maze.get(x,y-1).getLeft())
            {std::fill_n(&activeBuffer[y*Maze::TILESIZE*2], 4, LV_COLOR_WHITE);}
      else  {std::fill_n(&activeBuffer[y*Maze::TILESIZE*2], 4, LV_COLOR_BLACK);}
      // handle actual wall segments
      if (curblock.getLeft()) {std::fill_n(&activeBuffer[y*Maze::TILESIZE*2+4], Maze::TILESIZE*2-4, LV_COLOR_WHITE);}
      else                    {std::fill_n(&activeBuffer[y*Maze::TILESIZE*2+4], Maze::TILESIZE*2-4, LV_COLOR_BLACK);}
    }
    area.x1 = Maze::TILESIZE * x - 1;
    area.x2 = Maze::TILESIZE * x;
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, &activeBuffer[4]);
    activeBuffer = (activeBuffer==buf1) ? buf2 : buf1;  // switch buffer
  }

  // Print borders
  // don't need to worry about switching buffers here since buffer contents aren't changing
  std::fill_n(activeBuffer, 240, LV_COLOR_GRAY);
  for (int i = 0; i < 4; i++) {
    if (i==0)      {area.x1=0;   area.x2=239; area.y1=0;   area.y2=0;  } // top
    else if (i==1) {area.x1=0;   area.x2=239; area.y1=239; area.y2=239;} // bottom
    else if (i==2) {area.x1=0;   area.x2=0;   area.y1=0;   area.y2=239;} // left
    else if (i==3) {area.x1=239; area.x2=239; area.y1=0;   area.y2=239;} // right
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, activeBuffer);
  }
}


void WatchFaceMaze::DrawMazeSide(int16_t x, int16_t y, TileAttr side, lv_color_t wallcolor, lv_color_t bgcolor) {
  // convert right and down sides to up and left, makes rest of the code easier
  if (side == TileAttr::right) {
    x++;
    side = TileAttr::left;
  } else if (side == TileAttr::down) {
    y++;
    side = TileAttr::up;
  }

  // early exit if would print OOB
  if ((x == 0 && side == TileAttr::left) ||
      (x < 0) ||
      (x >= Maze::WIDTH) ||
      (y == 0 && side == TileAttr::up) ||
      (y < 0) ||
      (y >= Maze::HEIGHT))
    {return;}

  // prepare buffer
  activeBuffer = (activeBuffer==buf1) ? buf2 : buf1;
  std::fill_n(activeBuffer, 16, maze.getSide(x, y, side) ? wallcolor : bgcolor);
  lv_area_t area;

  // figure where to print
  if (side == TileAttr::up) {
    // drawing top side
    area.x1 = 10*x + 1;
    area.x2 = 10*x + 8;
    area.y1 = 10*y - 1;
    area.y2 = 10*y;
  } else if (side == TileAttr::left) {
    // drawing left side
    area.x1 = 10*x - 1;
    area.x2 = 10*x;
    area.y1 = 10*y + 1;
    area.y2 = 10*y + 8;
  }

  // print to screen
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  lvgl.FlushDisplay(&area, activeBuffer);
}


void WatchFaceMaze::UpdateBatteryDisplay(bool forceRedraw) {
  batteryPercent = batteryController.PercentRemaining();
  charging = batteryController.IsCharging();
  if (forceRedraw || batteryPercent.IsUpdated() || charging.IsUpdated()) {
    // need to redraw battery stuff
    activeBuffer = (activeBuffer==buf1) ? buf2 : buf1;  // switch buffer

    // number of pixels between top of indicator and fill line. rounds up, so 0% is 24px but 1% is 23px
    uint8_t fillLevel = 24 - ((uint16_t)(batteryPercent.Get()) * 24) / 100;
    lv_area_t area = {223,3,236,26};

    // battery body color - green >25%, orange >10%, red <=10%. Charging always makes it yellow.
    lv_color_t batteryBodyColor;
    if (charging.Get()) {batteryBodyColor = LV_COLOR_YELLOW;}
    else if (batteryPercent.Get() > 25) {batteryBodyColor = LV_COLOR_GREEN;}
    else if (batteryPercent.Get() > 10) {batteryBodyColor = LV_COLOR_ORANGE;}
    else {batteryBodyColor = LV_COLOR_RED;}

    // battery top color (upper gray section) - gray normally, light blue when charging, light red at <=10% charge
    lv_color_t batteryTopColor;
    if (charging.Get()) {batteryTopColor = LV_COLOR_MAKE(0x80,0x80,0xC0);}
    else if (batteryPercent.Get() <= 10) {batteryTopColor = LV_COLOR_MAKE(0xC0,0x80,0x80);}
    else {batteryTopColor = LV_COLOR_GRAY;}

    // actually fill the buffer with the chosen colors and print it
    std::fill_n(activeBuffer, fillLevel*14, batteryTopColor);
    std::fill_n((activeBuffer+fillLevel*14), (24-fillLevel)*14, batteryBodyColor);
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, activeBuffer);
  }
}


void WatchFaceMaze::UpdateBleDisplay(bool forceRedraw) {
  bleConnected = bleController.IsConnected();
  if (forceRedraw || bleConnected.IsUpdated()) {
    // need to redraw BLE indicator
    activeBuffer = (activeBuffer==buf1) ? buf2 : buf1;  // switch buffer

    lv_area_t area = {213,3,216,26};
    std::fill_n(activeBuffer, 96, (bleConnected.Get() ? LV_COLOR_BLUE : LV_COLOR_GRAY));

    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, activeBuffer);
  }
}


void WatchFaceMaze::ClearIndicators() {
  activeBuffer = (activeBuffer==buf1) ? buf2 : buf1;  // switch buffer
  lv_area_t area;
  std::fill_n(activeBuffer, 24*14, LV_COLOR_BLACK);

  // battery indicator
  area.x1=223; area.y1=3; area.x2=236; area.y2=26;
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  lvgl.FlushDisplay(&area, activeBuffer);

  // BLE indicator
  area.x1=213; area.y1=3; area.x2=216; area.y2=26;
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  lvgl.FlushDisplay(&area, activeBuffer);
}


void WatchFaceMaze::ClearConfetti() {
  // prevent superfluous calls
  if (!confettiActive) {return;}

  // clear all particles and reset state
  for (const ConfettiParticle &particle : confettiArr) {
    DrawMazeSide(particle.tilex, particle.tiley, TileAttr(particle.side), LV_COLOR_WHITE, LV_COLOR_BLACK);
  }
  confettiActive = false;
}


void WatchFaceMaze::ProcessConfetti() {
  // and draw all the confetti
  // flag "done" stays true if all step() calls stated that the particle was done, otherwise it goes false
  bool done = true;
  for (ConfettiParticle &particle : confettiArr) {
    int16_t oldx = particle.tilex;
    int16_t oldy = particle.tiley;
    uint8_t oldside = particle.side;
    // if any step() calls return false (i.e. not finished), done gets set to false as well
    done = particle.step() && done;
    // need to redraw?
    if (oldx != particle.tilex || oldy != particle.tiley || oldside != particle.side) {
      DrawMazeSide(oldx, oldy, TileAttr(oldside), LV_COLOR_WHITE, LV_COLOR_BLACK);
      DrawMazeSide(particle.tilex, particle.tiley, TileAttr(particle.side), LV_COLOR_RED, LV_COLOR_MAKE(0x80,0,0));
    }
  }

  // handle done flag
  // should only set confettiActive to false, since all confetti will have been cleared as it moved out of frame
  if (done) {
    confettiActive = false;
  }
}