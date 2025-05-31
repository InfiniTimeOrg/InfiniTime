#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Adder.h"
#include <cstdlib>   // For std::rand
#include <algorithm> // For std::max

using namespace Pinetime::Applications::Screens;

Adder::Adder(Pinetime::Components::LittleVgl& lvgl, Controllers::FS& fs) : lvgl(lvgl), filesystem(fs) {
  InitializeGame();
}

Adder::~Adder() {
  CleanUp();
}

void Adder::InitializeGame() {
  LoadGame();

  tileBuffer = new lv_color_t[TileSize * TileSize];
  std::fill(tileBuffer, tileBuffer + TileSize * TileSize, LV_COLOR_WHITE);

  displayHeight = LV_VER_RES;
  displayWidth = LV_HOR_RES;

  fieldHeight = displayHeight / TileSize - 2;
  fieldWidth = displayWidth / TileSize - 1;
  fieldOffsetHorizontal = (displayWidth - fieldWidth * TileSize) / 2;
  fieldOffsetVertical = (displayHeight - fieldHeight * TileSize) / 2 + (TileSize + 0.5) / 2;

  fieldSize = fieldWidth * fieldHeight;
  field = new AdderField[fieldSize];

  InitializeBody();
  CreateLevel();

  refreshTask = lv_task_create(
    [](lv_task_t* task) {
      auto* adder = static_cast<Adder*>(task->user_data);
      adder->Refresh();
    },
    AdderDelayInterval,
    LV_TASK_PRIO_MID,
    this);

  appReady = false;
  vTaskDelay(20);
}

void Adder::CleanUp() {
  delete[] field;
  delete[] tileBuffer;
  if (refreshTask) {
    lv_task_del(refreshTask);
  }
  lv_obj_clean(lv_scr_act());
}

void Adder::LoadGame() {
  lfs_file file;

  if (filesystem.FileOpen(&file, GameSavePath, LFS_O_RDONLY) == LFS_ERR_OK) {
    filesystem.FileRead(&file, reinterpret_cast<uint8_t*>(&data), sizeof(AdderSave));
    filesystem.FileClose(&file);

    if (data.Version != AdderVersion) {
      data = AdderSave();
    } else {
      highScore = std::max(data.HighScore, highScore);
    }
  } else {
    data = AdderSave();
    filesystem.DirCreate("/games");
    filesystem.DirCreate("/games/adder");
    SaveGame();
  }
}

void Adder::SaveGame() {
  lfs_file file;

  if (filesystem.FileOpen(&file, GameSavePath, LFS_O_WRONLY | LFS_O_CREAT) == LFS_ERR_OK) {
    filesystem.FileWrite(&file, reinterpret_cast<uint8_t*>(&data), sizeof(AdderSave));
    filesystem.FileClose(&file);
  }
}

void Adder::ResetGame() {
  GameOver();
  appReady = false;
  highScore = std::max(highScore, static_cast<unsigned int>(adderBody.size() - 2));
  data.HighScore = highScore;
  SaveGame();

  CreateLevel();
  InitializeBody();
  UpdateScore(0);
  FullRedraw();
}

void Adder::InitializeBody() {
  adderBody.clear();

  unsigned int startPosition = (fieldHeight / 2) * fieldWidth + fieldWidth / 2 + 2;
  adderBody = {startPosition, startPosition - 1};

  currentDirection = 1; // Start moving to the right
  prevDirection = currentDirection;
}

void Adder::CreateLevel() {
  for (unsigned int i = 0; i < fieldSize; ++i) {
    unsigned int x = i % fieldWidth;
    unsigned int y = i / fieldWidth;
    if (y == 0 || y == fieldHeight - 1 || x == 0 || x == fieldWidth - 1) {
      field[i] = AdderField::SOLID;
    } else {
      field[i] = AdderField::BLANK;
    }
  }
}

void Adder::CreateFood() {
  blanks.clear();
  for (unsigned int i = 0; i < fieldSize; ++i) {
    if (field[i] == AdderField::BLANK) {
      blanks.push_back(i);
    }
  }

  if (!blanks.empty()) {
    unsigned int randomIndex = std::rand() % blanks.size();
    field[blanks[randomIndex]] = AdderField::FOOD;
    UpdateSingleTile(blanks[randomIndex] % fieldWidth, blanks[randomIndex] / fieldWidth, LV_COLOR_GREEN);
  }
}

bool Adder::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeLeft:
      currentDirection = -1;
      break;
    case TouchEvents::SwipeUp:
      currentDirection = -fieldWidth;
      break;
    case TouchEvents::SwipeDown:
      currentDirection = fieldWidth;
      break;
    case TouchEvents::SwipeRight:
      currentDirection = 1;
      break;
    case TouchEvents::LongTap:
      FullRedraw();
    default:
      break;
  }

  // Prevent the adder from directly reversing direction
  if (prevDirection == -currentDirection) {
    currentDirection = -currentDirection;
  }

  // Update previous direction if it differs
  if (currentDirection != prevDirection) {
    prevDirection = currentDirection;
  }

  return true; // Return true to indicate the touch event was handled
}

void Adder::UpdatePosition() {
  unsigned int newHead = adderBody.front() + currentDirection;
  Adder::MoveConsequence result = CheckMove();

  switch (result) {
    case Adder::MoveConsequence::DEATH:
      ResetGame();
      return;

    case Adder::MoveConsequence::EAT:
      adderBody.push_front(newHead);
      CreateFood();
      UpdateScore(adderBody.size() - 2);
      break;

    case Adder::MoveConsequence::MOVE:
      adderBody.pop_back();
      adderBody.push_front(newHead);
      break;
  }

  field[adderBody.front()] = AdderField::BODY;
  field[adderBody.back()] = AdderField::BLANK;
}

Adder::MoveConsequence Adder::CheckMove() const {
  unsigned int newHead = adderBody.front() + currentDirection;
  if (newHead >= fieldSize) {
    return Adder::MoveConsequence::DEATH;
  }

  switch (field[newHead]) {
    case AdderField::BLANK:
      return Adder::MoveConsequence::MOVE;
    case AdderField::FOOD:
      return Adder::MoveConsequence::EAT;
    default:
      return Adder::MoveConsequence::DEATH;
  }
}

void Adder::Refresh() {
  if (!appReady) {
    FullRedraw();
    CreateFood();
    vTaskDelay(1); // Required to let the OS draw the tile completely
    UpdateScore(0);
    vTaskDelay(1); // Required to let the OS draw the tile completely
    appReady = true;
  } else {
    UpdatePosition();
    UpdateSingleTile(adderBody.front() % fieldWidth, adderBody.front() / fieldWidth, LV_COLOR_YELLOW);
    vTaskDelay(1); // Required to let the OS draw the tile completely
    UpdateSingleTile(adderBody.back() % fieldWidth, adderBody.back() / fieldWidth, LV_COLOR_BLACK);
    vTaskDelay(1); // Required to let the OS draw the tile completely
  }
}

void Adder::FullRedraw() {
  for (unsigned int x = 0; x < fieldWidth; ++x) {
    for (unsigned int y = 0; y < fieldHeight; ++y) {
      lv_color_t color;
      switch (field[y * fieldWidth + x]) {
        case AdderField::BODY:
          color = LV_COLOR_YELLOW;
          break;
        case AdderField::SOLID:
          color = LV_COLOR_WHITE;
          break;
        case AdderField::FOOD:
          color = LV_COLOR_GREEN;
          break;
        default:
          color = LV_COLOR_BLACK;
          break;
      }
      UpdateSingleTile(x, y, color);
      vTaskDelay(1); // Required to let the OS draw the tile completely
    }
  }
}

void Adder::UpdateSingleTile(unsigned int x, unsigned int y, lv_color_t color) {
  std::fill(tileBuffer, tileBuffer + TileSize * TileSize, color);
  lv_area_t area {.x1 = static_cast<lv_coord_t>(x * TileSize + fieldOffsetHorizontal),
                  .y1 = static_cast<lv_coord_t>(y * TileSize + fieldOffsetVertical),
                  .x2 = static_cast<lv_coord_t>(x * TileSize + fieldOffsetHorizontal + TileSize - 1),
                  .y2 = static_cast<lv_coord_t>(y * TileSize + fieldOffsetVertical + TileSize - 1)};

  lvgl.FlushDisplay(&area, tileBuffer);
}

void Adder::GameOver() {
  unsigned int digits[] = {7, 0, 5, 3}; // Digits forming the "GAME OVER" display

  // Determine offset based on field dimensions
  unsigned int offset = fieldOffsetHorizontal > fieldOffsetVertical ? fieldOffsetHorizontal : fieldOffsetVertical;

  // Render "GAME OVER" animation
  for (unsigned int r = 3 * offset; r < displayWidth - 4 * offset; r += 16) {
    for (unsigned int i = 0; i < 4; i++) {
      for (unsigned int j = 0; j < 64; j++) {
        // Map font bits into the display buffer
        digitBuffer[63 - j] = (DigitFont[digits[i]][j / 8] & 1 << j % 8)
                                ? LV_COLOR_WHITE
                                : LV_COLOR_BLACK; // Bitmagic to rotate the Digits to look like Letters
      }

      lv_area_t area;
      area.x1 = r + 8 * i;
      area.y1 = r;
      area.x2 = area.x1 + 7;
      area.y2 = area.y1 + 7;

      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
      lvgl.FlushDisplay(&area, digitBuffer);
      vTaskDelay(1); // Required to let the OS draw the tile completely
    }
  }
}

void Adder::UpdateScore(unsigned int score) {
  // Extract individual digits of the score
  unsigned int digits[] = {0, score % 10, (score % 100 - score % 10) / 10, (score - score % 100) / 100};

  // Render the score
  for (unsigned int i = 0; i < 4; i++) {
    for (unsigned int j = 0; j < 64; j++) {
      // Map font bits into the display buffer (using bit manipulation)
      digitBuffer[j] = (DigitFont[digits[i]][j / 8] & (1 << (j % 8))) ? LV_COLOR_WHITE : LV_COLOR_BLACK;
    }

    lv_area_t area;
    area.x1 = displayWidth - 16 - 8 * i; // Adjust X to display digits
    area.y1 = 4;                         // Y-offset for Score
    area.x2 = area.x1 + 7;
    area.y2 = area.y1 + 7;

    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, digitBuffer);
    vTaskDelay(20); // Small delay to allow display refresh
  }

  // Update the high score if necessary
  unsigned int highScoreToWrite = (highScore > score) ? highScore : score;
  unsigned int highScoreDigits[] = {0,
                                    highScoreToWrite % 10,
                                    (highScoreToWrite % 100 - highScoreToWrite % 10) / 10,
                                    (highScoreToWrite - highScoreToWrite % 100) / 100};

  // Render the high score
  for (unsigned int i = 0; i < 4; i++) {
    for (unsigned int j = 0; j < 64; j++) {
      // Map font bits into the display buffer
      digitBuffer[j] = (DigitFont[highScoreDigits[i]][j / 8] & (1 << (j % 8))) ? LV_COLOR_WHITE : LV_COLOR_BLACK;
    }

    lv_area_t area;
    area.x1 = 40 - 8 * i; // Adjust X to display digits
    area.y1 = 4;          // Y-offset for High Score
    area.x2 = area.x1 + 7;
    area.y2 = area.y1 + 7;

    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, digitBuffer);
    vTaskDelay(20); // Small delay to allow display refresh
  }

  // Save the high score if it has changed
  highScore = highScoreToWrite;
}
