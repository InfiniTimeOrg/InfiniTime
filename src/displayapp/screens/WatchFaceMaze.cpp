#include "displayapp/screens/WatchFaceMaze.h"

using namespace Pinetime::Applications::Screens;

// Despite being called Maze, this really is only a relatively simple wrapper for the specialized
// (fake) 2d array on which the maze structure is built. It should only have manipulations for
// the structure; generating and printing should be handled elsewhere.
Maze::Maze() {
  std::fill_n(mazeMap, FLATSIZE, 0);
}

// Only returns 4 bits (since that's all that's stored)
// Returns set walls but unset flags in case of out of bounds access
MazeTile Maze::Get(const coord_t x, const coord_t y) const {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
    return MazeTile(MazeTile::UPMASK | MazeTile::LEFTMASK);
  }
  return Get((y * WIDTH) + x);
}

MazeTile Maze::Get(const int32_t index) const {
  if (index < 0 || index / 2 >= FLATSIZE) {
    return MazeTile(MazeTile::UPMASK | MazeTile::LEFTMASK);
  }
  // odd means right (low) nibble, even means left (high) nibble
  if (index % 2 == 1) {
    return MazeTile(mazeMap[index / 2] & 0b00001111);
  }
  return MazeTile(mazeMap[index / 2] >> 4);
}

// Only stores the low 4 bits of the value
// If out of bounds, does nothing
void Maze::Set(const coord_t x, const coord_t y, const MazeTile tile) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
    return;
  }
  Set((y * WIDTH) + x, tile);
}

void Maze::Set(const int32_t index, const MazeTile tile) {
  if (index < 0 || index / 2 >= FLATSIZE) {
    return;
  }
  // odd means right (low) nibble, even means left (high) nibble
  if (index % 2 == 1) {
    mazeMap[index / 2] = (mazeMap[index / 2] & 0b11110000) | tile.map;
  } else {
    mazeMap[index / 2] = (mazeMap[index / 2] & 0b00001111) | tile.map << 4;
  }
}

// For quickly manipulating. Also allows better abstraction by allowing setting of down and right sides.
// Silently does nothing if given invalid values.
void Maze::SetAttr(const int32_t index, const TileAttr attr, const bool value) {
  switch (attr) {
    case Up:
      Set(index, Get(index).SetUp(value));
      break;
    case Down:
      Set(index + WIDTH, Get(index + WIDTH).SetUp(value));
      break;
    case Left:
      Set(index, Get(index).SetLeft(value));
      break;
    case Right:
      Set(index + 1, Get(index + 1).SetLeft(value));
      break;
    case FlagEmpty:
      Set(index, Get(index).SetFlagEmpty(value));
      break;
    case FlagGen:
      Set(index, Get(index).SetFlagGen(value));
      break;
  }
}

void Maze::SetAttr(const coord_t x, const coord_t y, const TileAttr attr, const bool value) {
  SetAttr((y * WIDTH) + x, attr, value);
}

bool Maze::GetTileAttr(const int32_t index, const TileAttr attr) const {
  switch (attr) {
    case Up:
      return Get(index).GetUp();
    case Down:
      return Get(index + WIDTH).GetUp();
    case Left:
      return Get(index).GetLeft();
    case Right:
      return Get(index + 1).GetLeft();
    case FlagEmpty:
      return Get(index).GetFlagEmpty();
    case FlagGen:
      return Get(index).GetFlagGen();
  }
  return false;
}

bool Maze::GetTileAttr(const coord_t x, const coord_t y, const TileAttr attr) const {
  return GetTileAttr((y * WIDTH) + x, attr);
}

// Only operates on the low 4 bits of the uint8_t.
// Only sets the bits from the value that are also on in the mask, rest are left alone
// e.g. existing = 1010, value = 0001, mask = 0011, then result = 1001
// (mask defaults to 0xFF which keeps all bits)
void Maze::Fill(uint8_t value, uint8_t mask) {
  value = value & 0b00001111;
  value |= value << 4;

  if (mask >= 0x0F) {
    // mask includes all bits, simply use fill_n
    std::fill_n(mazeMap, FLATSIZE, value);

  } else {
    // included a mask
    mask = mask & 0b00001111;
    mask |= mask << 4;
    value = value & mask; // preprocess mask for value
    mask = ~mask;         // this inverted mask will be applied to the existing value in mazeMap
    for (uint8_t& mapItem : mazeMap) {
      mapItem = (mapItem & mask) + value;
    }
  }
}

inline void Maze::Fill(const MazeTile tile, const uint8_t mask) {
  Fill(tile.map, mask);
}

// Paste a set of tiles into the given coords.
void Maze::PasteMazeSeed(const coord_t x1, const coord_t y1, const coord_t x2, const coord_t y2, const uint8_t toPaste[]) {
  // Assumes a maze with empty flags all true, and all walls present
  int32_t flatCoord = 0; // the position in the array (inside the byte, so index 1 would be mask 0b00110000 in the first byte)
  for (coord_t y = y1; y <= y2; y++) {
    for (coord_t x = x1; x <= x2; x++) {
      // working holds the target wall (bit 2 for left wall, bit 1 for up wall)
      const uint8_t working = (toPaste[flatCoord / 4] & (0b11 << ((3 - (flatCoord % 4)) * 2))) >> ((3 - (flatCoord % 4)) * 2);

      // handle left wall
      if (!(bool) (working & 0b10)) {
        SetAttr(x, y, TileAttr::Left, false);
        SetAttr(x, y, TileAttr::FlagEmpty, false);
        if (x > 0) {
          SetAttr(x - 1, y, TileAttr::FlagEmpty, false);
        }
      }

      // handle up wall
      if (!(bool) (working & 0b01)) {
        SetAttr(x, y, TileAttr::Up, false);
        SetAttr(x, y, TileAttr::FlagEmpty, false);
        if (y > 0) {
          SetAttr(x, y - 1, TileAttr::FlagEmpty, false);
        }
      }

      flatCoord++;
    }
  }
}

bool ConfettiParticle::Step() {
  // first apply gravity (only to y), then dampening, then apply velocity to position.
  xVel *= DAMPING_FACTOR;
  xPos += xVel;

  yVel += GRAVITY;
  yVel *= DAMPING_FACTOR;
  yPos += yVel;

  // return true if particle is finished (went OOB (ignore top; particle can still fall down))
  return xPos < 0 || xPos > 240 || yPos > 240;
}

void ConfettiParticle::Reset(MazeRNG& prng) {
  // always start at bottom middle
  xPos = 120;
  yPos = 240;

  // velocity in pixels/tick
  const float velocity = ((float) prng.Rand(MIN_START_VELOCITY * 100, MAX_START_VELOCITY * 100)) / 100;
  // angle, in radians, for going up at the chosen degree angle
  const float angle = ((float) prng.Rand(0, MAX_START_ANGLE * 2) - MAX_START_ANGLE + 90) * (std::numbers::pi_v<float> / 180);

  xVel = std::cos(angle) * velocity * START_X_COMPRESS;
  yVel = -std::sin(angle) * velocity;

  // Low 3 bits represent red, green, and blue. Also don't allow all three off or all three on at once.
  // Effectively choose any max saturation color except black or white.
  const uint8_t colorBits = prng.Rand(1, 6);
  color = LV_COLOR_MAKE((colorBits & 0b001) * 0xFF, ((colorBits & 0b010) >> 1) * 0xFF, ((colorBits & 0b100) >> 2) * 0xFF);
}

WatchFaceMaze::WatchFaceMaze(Pinetime::Components::LittleVgl& lvgl,
                             Controllers::DateTime& dateTimeController,
                             Controllers::Settings& settingsController,
                             Controllers::MotorController& motor,
                             const Controllers::Battery& batteryController,
                             const Controllers::Ble& bleController)
  : dateTimeController{dateTimeController},
    settingsController{settingsController},
    motor{motor},
    batteryController{batteryController},
    bleController{bleController},
    lvgl{lvgl},
    maze{Maze()},
    prng{MazeRNG()} {

  // set it to be in the past so it always takes two clicks to go to the secret, even if you're fast
  lastLongClickTime = xTaskGetTickCount() - doubleDoubleClickDelay;

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  // Calling Refresh() here causes all sorts of issues, rely on task to refresh instead
}

WatchFaceMaze::~WatchFaceMaze() {
  lv_obj_clean(lv_scr_act());
  lv_task_del(taskRefresh);
}

void WatchFaceMaze::Refresh() {

  // handle everything related to refreshing and printing stuff to the screen
  HandleMazeRefresh();

  // handle confetti printing
  // yeah it's not very pretty how this is hanging out in the refresh() function but I don't want to modify anything related
  // to printing in the touch interrupts
  HandleConfettiRefresh();
}

void WatchFaceMaze::HandleMazeRefresh() {
  // convert time to minutes and update if needed
  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());

  // Refresh if it's needed by some other component, a minute has passed on the watchface, or if generation has paused.
  if (screenRefreshRequired || (currentState == Displaying::WatchFace && currentDateTime.IsUpdated()) || pausedGeneration) {

    // if generation wasn't paused (i.e. doing a ground up maze gen), set everything up
    if (!pausedGeneration) {
      // only reseed PRNG if got here by the minute rolling over
      if (!screenRefreshRequired) {
        prng.Seed(currentDateTime.Get().time_since_epoch().count());
      }
      InitializeMaze();
      SeedMaze();
    }

    // always need to run GenerateMaze() when refreshing. This is a maze watchface after all.
    GenerateMaze();

    // only finalize and draw once maze is fully generated (not paused)
    if (!pausedGeneration) {
      ForceValidMaze();
      if (currentState != Displaying::WatchFace) {
        ClearIndicators();
      }
      DrawMaze();
      screenRefreshRequired = false;
      // if on watchface, also add indicators for BLE and battery
      if (currentState == Displaying::WatchFace) {
        UpdateBatteryDisplay(true);
        UpdateBleDisplay(true);
      }
    }
  }

  // update battery and ble displays if on main watchface
  if (currentState == Displaying::WatchFace) {
    UpdateBatteryDisplay();
    UpdateBleDisplay();
  }
}

void WatchFaceMaze::HandleConfettiRefresh() {
  // initialize confetti if tapped on autism creature
  if (initConfetti) {
    ClearConfetti();
    for (ConfettiParticle& particle : confettiArr) {
      particle.Reset(prng);
    }
    confettiActive = true;
    initConfetti = false;
  }
  // update confetti if needed
  if (confettiActive) {
    if (currentState != Displaying::AutismCreature) {
      // remove confetti if went to a different display
      ClearConfetti();
      confettiActive = false;
    } else {
      // still on autism creature display, step confetti
      ProcessConfetti();
    }
  }
}

void WatchFaceMaze::UpdateBatteryDisplay(const bool forceRedraw) {
  batteryPercent = batteryController.PercentRemaining();
  charging = batteryController.IsCharging();
  if (forceRedraw || batteryPercent.IsUpdated() || charging.IsUpdated()) {
    // need to redraw battery stuff
    SwapActiveBuffer();

    // number of pixels between top of indicator and fill line. rounds up, so 0% is 24px but 1% is 23px
    const uint8_t fillLevel = 24 - ((uint16_t) (batteryPercent.Get()) * 24) / 100;
    constexpr lv_area_t area = {223, 3, 236, 26};

    // battery body color - green >25%, orange >10%, red <=10%. Charging always makes it yellow.
    lv_color_t batteryBodyColor;
    if (charging.Get()) {
      batteryBodyColor = LV_COLOR_YELLOW;
    } else if (batteryPercent.Get() > 25) {
      batteryBodyColor = LV_COLOR_GREEN;
    } else if (batteryPercent.Get() > 10) {
      batteryBodyColor = LV_COLOR_ORANGE;
    } else {
      batteryBodyColor = LV_COLOR_RED;
    }

    // battery top color (upper section) - gray normally, light blue when charging, light red at <=10% charge
    lv_color_t batteryTopColor;
    if (charging.Get()) {
      batteryTopColor = LV_COLOR_MAKE(0x80, 0x80, 0xC0);
    } else if (batteryPercent.Get() <= 10) {
      batteryTopColor = LV_COLOR_MAKE(0xC0, 0x80, 0x80);
    } else {
      batteryTopColor = LV_COLOR_GRAY;
    }

    // actually fill the buffer with the chosen colors and print it
    std::fill_n(activeBuffer, fillLevel * 14, batteryTopColor);
    std::fill_n(activeBuffer + (fillLevel * 14), (24 - fillLevel) * 14, batteryBodyColor);
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, activeBuffer);
  }
}

void WatchFaceMaze::UpdateBleDisplay(const bool forceRedraw) {
  bleConnected = bleController.IsConnected();
  if (forceRedraw || bleConnected.IsUpdated()) {
    // need to redraw BLE indicator
    SwapActiveBuffer();

    constexpr lv_area_t area = {213, 3, 216, 26};
    std::fill_n(activeBuffer, 96, (bleConnected.Get() ? LV_COLOR_BLUE : LV_COLOR_GRAY));

    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, activeBuffer);
  }
}

void WatchFaceMaze::ClearIndicators() {
  SwapActiveBuffer();
  lv_area_t area;
  std::fill_n(activeBuffer, 24 * 14, LV_COLOR_BLACK);

  // battery indicator
  area.x1 = 223;
  area.y1 = 3;
  area.x2 = 236;
  area.y2 = 26;
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  lvgl.FlushDisplay(&area, activeBuffer);

  // BLE indicator
  area.x1 = 213;
  area.y1 = 3;
  area.x2 = 216;
  area.y2 = 26;
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  lvgl.FlushDisplay(&area, activeBuffer);
}

bool WatchFaceMaze::OnTouchEvent(const TouchEvents event) {
  // if generation is paused, let it continue working on that. This should really never trigger.
  if (pausedGeneration) {
    return false;
  }

  // Interrupts never seem to overlap (instead they're queued) so I don't have to worry about anything happening while one of
  // these handlers are running

  switch (event) {
    case Pinetime::Applications::TouchEvents::LongTap:
      return HandleLongTap();
    case Pinetime::Applications::TouchEvents::Tap:
      return HandleTap();
    case Pinetime::Applications::TouchEvents::SwipeUp:
      return HandleSwipe(0);
    case Pinetime::Applications::TouchEvents::SwipeRight:
      return HandleSwipe(1);
    case Pinetime::Applications::TouchEvents::SwipeDown:
      return HandleSwipe(2);
    case Pinetime::Applications::TouchEvents::SwipeLeft:
      return HandleSwipe(3);
    default:
      return false;
  }
}

// allow pushing the button to go back to the watchface
bool WatchFaceMaze::OnButtonPushed() {
  if (currentState != Displaying::WatchFace) {
    screenRefreshRequired = true;
    currentState = Displaying::WatchFace;
    // set lastLongClickTime to be in the past so it always needs two long taps to get back to blank, even if you're fast
    lastLongClickTime = xTaskGetTickCount() - doubleDoubleClickDelay;
    return true;
  }
  return false;
}

bool WatchFaceMaze::HandleLongTap() {
  if (currentState == Displaying::WatchFace) {
    // On watchface; either refresh maze or go to blank state
    if (xTaskGetTickCount() - lastLongClickTime < doubleDoubleClickDelay) {
      // long tapped twice in sequence; switch to blank maze
      currentState = Displaying::Blank;
      screenRefreshRequired = true;
      std::fill_n(currentCode, sizeof(currentCode), 255); // clear current code in preparation for code entry
    } else {
      // long tapped not in main watchface; go back to previous state
      screenRefreshRequired = true;
    }
    lastLongClickTime = xTaskGetTickCount();
    motor.RunForDuration(20);

  } else {
    // Not on watchface; go back to main watchface
    screenRefreshRequired = true;
    currentState = Displaying::WatchFace;
    // set lastLongClickTime to be in the past so it always needs two long taps to get back to blank, even if you're fast
    lastLongClickTime = xTaskGetTickCount() - doubleDoubleClickDelay;
    motor.RunForDuration(20);
  }

  // no situation where long tap doesn't get handled
  return true;
}

bool WatchFaceMaze::HandleTap() {
  // confetti must only display on autismcreature
  if (currentState != Displaying::AutismCreature) {
    return false;
  }
  // only need to set initConfetti, everything else is handled in functions called by refresh()
  initConfetti = true;
  return true;
}

bool WatchFaceMaze::HandleSwipe(const uint8_t direction) {
  // Don't handle any swipes on watchface
  if (currentState == Displaying::WatchFace) {
    return false;
  }

  // Add the new direction to the swipe list, dropping the last item
  for (unsigned int i = sizeof(currentCode) - 1; i > 0; i--) {
    currentCode[i] = currentCode[i - 1];
  }
  currentCode[0] = direction;

  // check if valid code has been entered
  // Displaying::WatchFace is used here simply as a dummy value, and it will never transition to that
  Displaying newState = Displaying::WatchFace;
  if (std::memcmp(currentCode, lossCode, sizeof(lossCode)) == 0) {
    newState = Displaying::Loss;
  } else if (std::memcmp(currentCode, amogusCode, sizeof(amogusCode)) == 0) {
    newState = Displaying::Amogus;
  } else if (std::memcmp(currentCode, autismCode, sizeof(autismCode)) == 0) {
    newState = Displaying::AutismCreature;
  } else if (std::memcmp(currentCode, foxCode, sizeof(foxCode)) == 0) {
    newState = Displaying::FoxGame;
  } else if (std::memcmp(currentCode, reminderCode, sizeof(reminderCode)) == 0) {
    newState = Displaying::GameReminder;
  } else if (std::memcmp(currentCode, pinetimeCode, sizeof(pinetimeCode)) == 0) {
    newState = Displaying::PineTime;
  }

  // only request a screen refresh if state has been updated
  if (newState != Displaying::WatchFace) {
    currentState = newState;
    screenRefreshRequired = true;
    motor.RunForDuration(10);
    std::fill_n(currentCode, sizeof(currentCode), 0xFF); // clear code
  }
  return true;
}

// Clear maze
void WatchFaceMaze::InitializeMaze() {
  maze.Fill(MazeTile().SetLeft(true).SetUp(true).SetFlagEmpty(true));
}

// seeds the maze with whatever the current state needs
void WatchFaceMaze::SeedMaze() {
  switch (currentState) {
    case Displaying::WatchFace:
      PutTime();
      break;
    case Displaying::Blank: {
      // seed maze with 4 tiles
      const coord_t randX = (coord_t) prng.Rand(0, 20);
      const coord_t randY = (coord_t) prng.Rand(3, 20);
      maze.PasteMazeSeed(randX, randY, randX + 3, randY, blankseed);
      break;
    }
    case Displaying::Loss:
      maze.PasteMazeSeed(2, 2, 22, 21, loss);
      break;
    case Displaying::Amogus:
      maze.PasteMazeSeed(3, 0, 21, 23, amogus);
      break;
    case Displaying::AutismCreature:
      maze.PasteMazeSeed(0, 2, 23, 22, autismCreature);
      break;
    case Displaying::FoxGame:
      maze.PasteMazeSeed(0, 1, 23, 22, foxGame);
      break;
    case Displaying::GameReminder:
      maze.PasteMazeSeed(0, 3, 23, 19, gameReminder);
      break;
    case Displaying::PineTime:
      maze.PasteMazeSeed(2, 0, 21, 23, pinetime);
      break;
  }
}

// Put time and date info on the screen.
void WatchFaceMaze::PutTime() {
  uint8_t hours = dateTimeController.Hours();
  uint8_t minutes = dateTimeController.Minutes();

  // modify hours to account for 12 hour format
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    // if 0am in 12 hour format, it's 12am
    if (hours == 0) {
      hours = 12;
    }
    // if after noon in 12 hour format, shift over by 12 hours
    if (hours > 12) {
      maze.PasteMazeSeed(18, 15, 22, 22, pm);
      hours -= 12;
    } else {
      maze.PasteMazeSeed(18, 15, 22, 22, am);
    }
  }

  // put time on screen
  maze.PasteMazeSeed(3, 1, 8, 10, numbers[hours / 10]);      // top left: hours major digit
  maze.PasteMazeSeed(10, 1, 15, 10, numbers[hours % 10]);    // top right: hours minor digit
  maze.PasteMazeSeed(3, 13, 8, 22, numbers[minutes / 10]);   // bottom left: minutes major digit
  maze.PasteMazeSeed(10, 13, 15, 22, numbers[minutes % 10]); // bottom right: minutes minor digit

  // reserve some space at the top right to put the battery and BLE indicators there
  maze.PasteMazeSeed(21, 0, 23, 2, indicatorSpace);
}

// Generates the maze around whatever it was seeded with
void WatchFaceMaze::GenerateMaze() {
  coord_t x = -1;
  coord_t y = -1;
  // task should only run for 3/4 the time it takes for the task to refresh.
  // Will go over; only checks once it's finished with current line. It won't go too far over though.
  uint32_t mazeGenStartTime = xTaskGetTickCount();

  while (true) {
    // find position to start generating a path from
    for (uint8_t i = 0; i < 30; i++) {
      x = (coord_t) prng.Rand(0, Maze::WIDTH - 1);
      y = (coord_t) prng.Rand(0, Maze::HEIGHT - 1);
      if (maze.GetTileAttr(x, y, TileAttr::FlagEmpty)) {
        break;
      } // found solution tile
      if (i == 29) {
        // failed all 30 attempts (this is inside the for loop for 'organization')
        // find solution tile slowly but guaranteed (scan over entire field and choose random valid tile)
        int count = 0;

        // count number of valid tiles
        for (int32_t j = 0; j < Maze::WIDTH * Maze::HEIGHT; j++) {
          if (maze.GetTileAttr(j, TileAttr::FlagEmpty)) {
            count++;
          }
        }

        // if no valid tiles are left, maze is done
        if (count == 0) {
          pausedGeneration = false;
          return;
        }

        // if execution gets here then maze gen is not done. select random index from valid tiles to start from
        // 'count' is now used as an index
        count = (coord_t) prng.Rand(1, count);
        for (int32_t j = 0; j < Maze::WIDTH * Maze::HEIGHT; j++) {
          if (maze.GetTileAttr(j, TileAttr::FlagEmpty)) {
            count--;
          }
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
    if (xTaskGetTickCount() - mazeGenStartTime > taskRefresh->period * 3 / 4) {
      pausedGeneration = true;
      return;
    }
  }
  // execution never gets here! it returns earlier in the function.
}

void WatchFaceMaze::GeneratePath(coord_t x, coord_t y) {
  // oldX, oldY are used in backtracking
  coord_t oldX;
  coord_t oldY;
  // which direction the cursor moved in
  uint8_t direction;
  while (true) {
    // set current tile to reflect that it's been worked on
    maze.SetAttr(x, y, TileAttr::FlagEmpty, false); // no longer empty
    maze.SetAttr(x, y, TileAttr::FlagGen, true);    // in generation
    oldX = x, oldY = y;                             // used in backtracking

    // move to next tile
    // the if statements are very scuffed, but they prevent turning around.
    while (true) {
      switch (direction = prng.Rand(0, 3)) {
        case 0: // moved up
          if (y <= 0 || !maze.GetTileAttr(x, y, TileAttr::Up)) {
            continue;
          }
          y -= 1;
          break;
        case 1: // moved left
          if (x <= 0 || !maze.GetTileAttr(x, y, TileAttr::Left)) {
            continue;
          }
          x -= 1;
          break;
        case 2: // moved down
          if (y >= Maze::HEIGHT - 1 || !maze.GetTileAttr(x, y, TileAttr::Down)) {
            continue;
          }
          y += 1;
          break;
        case 3: // moved right
          if (x >= Maze::WIDTH - 1 || !maze.GetTileAttr(x, y, TileAttr::Right)) {
            continue;
          }
          x += 1;
          break;
        default: // invalid, will never hit in normal operation
          std::abort();
      }
      break;
    }

    // moved to next tile, check if looped in on self
    if (!maze.GetTileAttr(x, y, TileAttr::FlagGen)) {

      // did NOT loop in on self, simply remove wall and move on
      switch (direction) {
        case 0:
          maze.SetAttr(x, y, TileAttr::Down, false);
          break; // moved up
        case 1:
          maze.SetAttr(x, y, TileAttr::Right, false);
          break; // moved left
        case 2:
          maze.SetAttr(x, y, TileAttr::Up, false);
          break; // moved down
        case 3:
          maze.SetAttr(x, y, TileAttr::Left, false);
          break; // moved right
        default: // invalid, will never hit in normal operation
          std::abort();
      }

      // if attached to main maze, path finished generating
      if (!maze.GetTileAttr(x, y, TileAttr::FlagEmpty)) {
        break;
      }

    } else {

      // DID loop in on self, track down and eliminate loop
      // targets are the coordinates of where it needs to backtrack to
      const coord_t targetX = x;
      const coord_t targetY = y;
      x = oldX, y = oldY;
      while (x != targetX || y != targetY) {
        if (y > 0 && (maze.GetTileAttr(x, y, TileAttr::Up) == false)) {
          // backtrack up
          maze.SetAttr(x, y, TileAttr::Up, true);
          maze.SetAttr(x, y, TileAttr::FlagGen, false);
          maze.SetAttr(x, y, TileAttr::FlagEmpty, true);
          y -= 1;
        } else if (x > 0 && (maze.GetTileAttr(x, y, TileAttr::Left) == false)) {
          // backtrack left
          maze.SetAttr(x, y, TileAttr::Left, true);
          maze.SetAttr(x, y, TileAttr::FlagGen, false);
          maze.SetAttr(x, y, TileAttr::FlagEmpty, true);
          x -= 1;
        } else if (y < Maze::HEIGHT - 1 && (maze.GetTileAttr(x, y, TileAttr::Down) == false)) {
          // backtrack down
          maze.SetAttr(x, y, TileAttr::Down, true);
          maze.SetAttr(x, y, TileAttr::FlagGen, false);
          maze.SetAttr(x, y, TileAttr::FlagEmpty, true);
          y += 1;
        } else if (x < Maze::WIDTH && (maze.GetTileAttr(x, y, TileAttr::Right) == false)) {
          // backtrack right
          maze.SetAttr(x, y, TileAttr::Right, true);
          maze.SetAttr(x, y, TileAttr::FlagGen, false);
          maze.SetAttr(x, y, TileAttr::FlagEmpty, true);
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
  maze.Fill(0, MazeTile::FLAGGENMASK);
}

// goes through the maze, finds disconnected segments and connects them
void WatchFaceMaze::ForceValidMaze() {
  // Crude maze-optimized flood fill: follow a path until can't move any more, then find some other location to follow from. repeat.
  // Once it's traversed all reachable tiles, checks if there are any tiles that have not been traversed. if there are, then find a border
  // between the traversed and non-traversed segments. poke a hole at one of these borders randomly.
  // Once the hole has been poked, more maze is reachable. continue this "fill-search then poke" scheme until the entire maze is accessible.
  // This function repurposes flaggen for traversed tiles, so it expects it to be false on all tiles (should be in normal control flow)

  // initialize cursor x and y to bottom right
  coord_t x = Maze::WIDTH - 1;
  coord_t y = Maze::HEIGHT - 1;
  while (true) {
    // sorry for using goto but this needs to be really nested and the components are too integrated to split out into functions...
  ForceValidMazeLoop:
    maze.SetAttr(x, y, TileAttr::FlagGen, true);
    // move cursor
    if (y > 0 && !maze.GetTileAttr(x, y, TileAttr::Up) && !maze.GetTileAttr(x, y - 1, TileAttr::FlagGen)) {
      y--;
    } else if (x < Maze::WIDTH - 1 && !maze.GetTileAttr(x, y, TileAttr::Right) && !maze.GetTileAttr(x + 1, y, TileAttr::FlagGen)) {
      x++;
    } else if (y < Maze::HEIGHT - 1 && !maze.GetTileAttr(x, y, TileAttr::Down) && !maze.GetTileAttr(x, y + 1, TileAttr::FlagGen)) {
      y++;
    } else if (x > 0 && !maze.GetTileAttr(x, y, TileAttr::Left) && !maze.GetTileAttr(x - 1, y, TileAttr::FlagGen)) {
      x--;
    } else {
      unsigned int pokeLocationCount = 0;
      // couldn't find any position to move to, need to set cursor to a different usable location
      for (coord_t proposedY = 0; proposedY < Maze::HEIGHT; proposedY++) {
        for (coord_t proposedX = 0; proposedX < Maze::WIDTH; proposedX++) {
          const bool ownState = maze.GetTileAttr(proposedX, proposedY, TileAttr::FlagGen);

          // if tile to the left is of a different traversal state (is traversed boundary)
          if (proposedX > 0 && (maze.GetTileAttr(proposedX - 1, proposedY, TileAttr::FlagGen) != ownState)) {
            // if found boundary AND can get to it, just continue working from here
            if (maze.GetTileAttr(proposedX, proposedY, TileAttr::Left) == false) {
              x = proposedX, y = proposedY;
              goto ForceValidMazeLoop;
            }
            pokeLocationCount++;
          }

          // if tile up is of a different traversal state (is traversed boundary)
          if (proposedY > 0 && (maze.GetTileAttr(proposedX, proposedY - 1, TileAttr::FlagGen) != ownState)) {
            // if found boundary AND can get to it, just continue working from here
            if (maze.GetTileAttr(proposedX, proposedY, TileAttr::Up) == false) {
              x = proposedX, y = proposedY;
              goto ForceValidMazeLoop;
            }
            pokeLocationCount++;
          }
        }
      }
      // finished scanning maze; there are no locations the cursor can be placed for it to continue scanning

      // if there are no walls that can be poked through to increase reachable area, maze is finished
      if (pokeLocationCount == 0) {
        return;
      }

      // if execution gets here, need to poke a hole.
      // choose a random poke location to poke a hole through. pokeLocationCount is now used as an index
      pokeLocationCount = (int) prng.Rand(1, pokeLocationCount);

      for (coord_t proposedY = 0; proposedY < Maze::HEIGHT; proposedY++) {
        for (coord_t proposedX = 0; proposedX < Maze::WIDTH; proposedX++) {
          // pretty much a copy of the previous code which FINDS poke locations, but now with the goal of actually doing the poking
          const bool ownState = maze.GetTileAttr(proposedX, proposedY, TileAttr::FlagGen);

          if (proposedX > 0 && (maze.GetTileAttr(proposedX - 1, proposedY, TileAttr::FlagGen) != ownState)) {
            pokeLocationCount--;
            // found the target poke location, poke and loop
            if (pokeLocationCount == 0) {
              maze.SetAttr(proposedX, proposedY, TileAttr::Left, false);
              x = proposedX, y = proposedY;
              goto ForceValidMazeLoop; // continue OUTSIDE loop
            }
          }

          // if tile up is of a different traversal state (is traversed boundary)
          if (proposedY > 0 && (maze.GetTileAttr(proposedX, proposedY - 1, TileAttr::FlagGen) != ownState)) {
            pokeLocationCount--;
            // found the target poke location, poke and loop
            if (pokeLocationCount == 0) {
              maze.SetAttr(proposedX, proposedY, TileAttr::Up, false);
              x = proposedX, y = proposedY;
              goto ForceValidMazeLoop; // continue processing
            }
          }
        }
      }
    }
    // done poking a hole in the maze to expand the reachable area
  }
}

void WatchFaceMaze::DrawMaze() {
  // this used to be nice code, but it was retrofitted to print offset by 1 pixel for a fancy border.
  // I'm not proud of the logic but it works.
  lv_area_t area;
  SwapActiveBuffer(); // who knows who used the buffer before this

  // Print horizontal lines
  // This doesn't bother with corners, those just get overwritten by the vertical lines
  area.x1 = 1;
  area.x2 = 238;
  for (coord_t y = 1; y < Maze::HEIGHT; y++) {
    for (coord_t x = 0; x < Maze::WIDTH; x++) {
      if (maze.Get(x, y).GetUp()) {
        std::fill_n(&activeBuffer[x * Maze::TILESIZE], Maze::TILESIZE, LV_COLOR_WHITE);
      } else {
        std::fill_n(&activeBuffer[x * Maze::TILESIZE], Maze::TILESIZE, LV_COLOR_BLACK);
      }
    }
    std::copy_n(activeBuffer, 238, &activeBuffer[238]);
    area.y1 = (Maze::TILESIZE * y) - 1;
    area.y2 = (Maze::TILESIZE * y);
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, activeBuffer);
    SwapActiveBuffer();
  }

  // Print vertical lines
  area.y1 = 1;
  area.y2 = 238;
  for (coord_t x = 1; x < Maze::WIDTH; x++) {
    for (coord_t y = 0; y < Maze::HEIGHT; y++) {
      MazeTile curblock = maze.Get(x, y);
      // handle corners: if any of the touching lines are present, add corner. else leave it black
      if (curblock.GetUp() || curblock.GetLeft() || maze.Get(x - 1, y).GetUp() || maze.Get(x, y - 1).GetLeft()) {
        std::fill_n(&activeBuffer[y * Maze::TILESIZE * 2], 4, LV_COLOR_WHITE);
      } else {
        std::fill_n(&activeBuffer[y * Maze::TILESIZE * 2], 4, LV_COLOR_BLACK);
      }
      // handle actual wall segments
      if (curblock.GetLeft()) {
        std::fill_n(&activeBuffer[(y * Maze::TILESIZE * 2) + 4], (Maze::TILESIZE * 2) - 4, LV_COLOR_WHITE);
      } else {
        std::fill_n(&activeBuffer[(y * Maze::TILESIZE * 2) + 4], (Maze::TILESIZE * 2) - 4, LV_COLOR_BLACK);
      }
    }
    area.x1 = (Maze::TILESIZE * x) - 1;
    area.x2 = (Maze::TILESIZE * x);
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, &activeBuffer[4]);
    SwapActiveBuffer();
  }

  // Print borders
  // don't need to worry about switching buffers here since buffer contents aren't changing
  std::fill_n(activeBuffer, 240, LV_COLOR_GRAY);
  for (int i = 0; i < 4; i++) {
    if (i == 0) {
      area.x1 = 0;
      area.x2 = 239;
      area.y1 = 0;
      area.y2 = 0;
    } // top
    else if (i == 1) {
      area.x1 = 0;
      area.x2 = 239;
      area.y1 = 239;
      area.y2 = 239;
    } // bottom
    else if (i == 2) {
      area.x1 = 0;
      area.x2 = 0;
      area.y1 = 0;
      area.y2 = 239;
    } // left
    else if (i == 3) {
      area.x1 = 239;
      area.x2 = 239;
      area.y1 = 0;
      area.y2 = 239;
    } // right
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, activeBuffer);
  }
}

void WatchFaceMaze::DrawTileInner(const coord_t x, const coord_t y, const lv_color_t color) {
  // early exit if would print OOB
  if (x < 0 || y < 0 || x > Maze::WIDTH - 1 || y > Maze::HEIGHT - 1) {
    return;
  }

  // prepare buffer
  SwapActiveBuffer();
  std::fill_n(activeBuffer, 64, color);
  lv_area_t area;

  // define bounds
  area.x1 = (Maze::TILESIZE * x) + 1;
  area.x2 = (Maze::TILESIZE * x) + (Maze::TILESIZE - 2);
  area.y1 = (Maze::TILESIZE * y) + 1;
  area.y2 = (Maze::TILESIZE * y) + (Maze::TILESIZE - 2);

  // print to screen
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  lvgl.FlushDisplay(&area, activeBuffer);
}

void WatchFaceMaze::ClearConfetti() {
  // prevent superfluous calls
  if (!confettiActive) {
    return;
  }

  // clear all particles and reset state
  for (const ConfettiParticle& particle : confettiArr) {
    DrawTileInner(particle.TileX(), particle.TileY(), LV_COLOR_BLACK);
  }
  confettiActive = false;
}

void WatchFaceMaze::ProcessConfetti() {
  // and draw all the confetti
  // flag "done" stays true if all step() calls stated that the particle was done, otherwise it goes false
  bool isDone = true;
  for (ConfettiParticle& particle : confettiArr) {
    const coord_t oldX = particle.TileX();
    const coord_t oldY = particle.TileY();
    // if any step() calls return false (i.e. not finished), isDone gets set to false as well
    isDone = particle.Step() && isDone;
    // need to redraw?
    if (oldX != particle.TileX() || oldY != particle.TileY()) {
      DrawTileInner(oldX, oldY, LV_COLOR_BLACK);
      DrawTileInner(particle.TileX(), particle.TileY(), particle.color);
    }
  }

  // handle done flag
  // should only set confettiActive to false, since all confetti will have been cleared as it moved out of frame
  if (isDone) {
    confettiActive = false;
  }
}