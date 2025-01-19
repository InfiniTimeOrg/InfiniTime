#define LV_MONOSERT
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Adder.h"
#include <cstdlib> //randr
#include "task.h"

using namespace Pinetime::Applications::Screens;

Adder::Adder(Pinetime::Components::LittleVgl& lvgl, Controllers::FS& fs) : lvgl {lvgl}, filesystem {fs} {

  AppReady = false;

  LoadGame();
  

  TileBufferSize = TileSize * TileSize;
  TileBuffer = new lv_color_t[TileBufferSize];

  DisplayHeight = LV_VER_RES;
  DisplayWidth = LV_HOR_RES;

  FieldHeight = DisplayHeight / TileSize - 2;
  FieldWidth = DisplayWidth / TileSize - 1;
  FieldOffsetHorizontal = (DisplayWidth - FieldWidth * TileSize) / 2;
  FieldOffsetVertical = (DisplayHeight - FieldHeight * TileSize) / 2 + (TileSize + 0.5) / 2;

  FieldSize = FieldWidth * FieldHeight;

  Field = new AdderField[FieldSize];

  InitBody();

  for (unsigned int ti = 0; ti < TileBufferSize; ti++)
    TileBuffer[ti] = LV_COLOR_WHITE;

  createLevel();

  taskRefresh = lv_task_create(RefreshTaskCallback, AdderDelayInterval, LV_TASK_PRIO_MID, this);
}

Adder::~Adder() {
  delete[] Field;
  delete[] TileBuffer;
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Adder::LoadGame(){
  lfs_file f;


  if (filesystem.FileOpen(&f, GameSavePath, LFS_O_RDONLY) == LFS_ERR_OK) {
    filesystem.FileRead(&f, reinterpret_cast<uint8_t*>(&Data), sizeof(AdderSave));
    filesystem.FileClose(&f);
    if(Data.Version!=AdderVersion){
      Data= AdderSave();
    }else
      HighScore = std::max(Data.HighScore,HighScore);
    
  }
  else{
    Data=AdderSave();
    filesystem.DirCreate("games");
    filesystem.DirCreate("games/adder");
    SaveGame();
  }
  Data.Version=AdderVersion;

}

void Adder::SaveGame(){
  lfs_file f;
  
  if (filesystem.FileOpen(&f, GameSavePath, LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK)
      return;

  filesystem.FileWrite(&f, reinterpret_cast<uint8_t*>(&Data), sizeof(AdderSave));
  filesystem.FileClose(&f);
}

void Adder::InitBody() {
  AdderBody.clear();
  unsigned int start_position = (FieldHeight / 2) * FieldWidth + FieldWidth / 2 + 2;
  unsigned int body[] = {start_position, start_position - 1};
  AdderBody.assign(body, body + 2);
}

void Adder::createLevel() {
  for (unsigned int i = 0; i < FieldSize; i++) {
    unsigned int x = i % FieldWidth;
    unsigned int y = i / FieldWidth;
    if (y == 0 || y == FieldHeight - 1 || x == 0 || x == FieldWidth - 1)
      Field[i] = SOLID;
    else
      Field[i] = BLANK;
  }
}

void Adder::GameOver() {
  unsigned int Digit[] = {7, 0, 5, 3};

  unsigned int Offset = FieldOffsetHorizontal > FieldOffsetVertical ? FieldOffsetHorizontal : FieldOffsetVertical;
  for (unsigned int r = 3 * Offset; r < DisplayWidth - 4 * Offset; r += 16) {
    for (unsigned int i = 0; i < 4; i++) {
      for (unsigned int j = 0; j < 64; j++)
        DigitBuffer[63 - j] =
          (DigitFont[Digit[i]][j / 8] & 1 << j % 8) ? LV_COLOR_WHITE : LV_COLOR_BLACK; // Bitmagic to map the font to an image array

      lv_area_t area;
      area.x1 = r + 8 * i;
      area.y1 = r;
      area.x2 = area.x1 + 7;
      area.y2 = area.y1 + 7;
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
      lvgl.FlushDisplay(&area, DigitBuffer);
    }
  }
  createLevel();
  AdderDirection = 1;
  InitBody();
  AppReady = false;

  if(HighScore > Data.HighScore)
    SaveGame();
}

bool Adder::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeLeft:
      AdderDirection = -1;
      break;
    case TouchEvents::SwipeUp:
      AdderDirection = -FieldWidth;
      break;
    case TouchEvents::SwipeDown:
      AdderDirection = +FieldWidth;
      break;
    case TouchEvents::SwipeRight:
      AdderDirection = 1;
      break;
    case TouchEvents::LongTap:
      FullReDraw();
      break;
    default:
      break;
  }
  if (prevAdderDirection == -AdderDirection)
    AdderDirection = -AdderDirection;

  if (AdderDirection != prevAdderDirection)
    prevAdderDirection = AdderDirection;
  return true;
}

MoveConsequence Adder::checkMove() {
  if (AdderBody.front() + AdderDirection < FieldSize) {
    if (Field[AdderBody.front() + AdderDirection] == BLANK)
      return MOVE;
    if (Field[AdderBody.front() + AdderDirection] == FOOD)
      return EAT;
  }

  return DEATH;
}

void Adder::updateScore(unsigned int Score) {

  unsigned int Digit[] = {0, Score % 10, (Score % 100 - Score % 10) / 10, (Score - Score % 100) / 100};

  // Print Score
  for (unsigned int i = 0; i < 4; i++) {
    for (unsigned int j = 0; j < 64; j++)
      DigitBuffer[j] = (DigitFont[Digit[i]][j / 8] & 1 << j % 8) ? LV_COLOR_WHITE : LV_COLOR_BLACK; 
                        // Bitmagic to map the font to an image array

    lv_area_t area;
    area.x1 = DisplayWidth - 16 - 8 * i;
    area.y1 = 4;
    area.x2 = area.x1 + 7;
    area.y2 = area.y1 + 7;
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, DigitBuffer);
    vTaskDelay(1); // Hack to give give the system time to refresh
  }

  // Check if HighScore changed
  unsigned int HScore = (HighScore > Score) ? HighScore : Score;
  unsigned int HS_Digit[] = {0, HScore % 10, (HScore % 100 - HScore % 10) / 10, (HScore - HScore % 100) / 100};
  // Print Highscore
  for (unsigned int i = 0; i < 4; i++) {
    for (unsigned int j = 0; j < 64; j++)
      DigitBuffer[j] = (DigitFont[HS_Digit[i]][j / 8] & 1 << j % 8) ? LV_COLOR_WHITE : LV_COLOR_BLACK; 
                        // Bitmagic to map the font to an image array

    lv_area_t area;
    area.x1 = 40 - 8 * i;
    area.y1 = 4;
    area.x2 = area.x1 + 7;
    area.y2 = area.y1 + 7;
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, DigitBuffer);
    vTaskDelay(1); // Hack to give give the system time to refresh
  }
  HighScore = HScore;
}

void Adder::createFood() {
  Blanks.clear();

  for (unsigned int i = 0; i < FieldSize; ++i)
    if (Field[i] == BLANK)
      Blanks.push_back(i);

  unsigned int pos = rand() % Blanks.size();

  Field[Blanks[pos]] = FOOD;
  updateSingleTile(Blanks[pos] % FieldWidth, Blanks[pos] / FieldWidth, LV_COLOR_GREEN);
}

void Adder::updatePosition() {

  Field[AdderBody.front()] = BODY;
  Field[AdderBody.back()] = BLANK;

  switch (checkMove()) {

    case DEATH:
      GameOver();
      break;

    case EAT:
      AdderBody.push_front(AdderBody.front() + AdderDirection);
      createFood();
      updateScore(AdderBody.size() - 2);
      break;

    case MOVE:
      AdderBody.pop_back();
      AdderBody.push_front(AdderBody.front() + AdderDirection);
      break;
  }
}

void Adder::FullReDraw() {
  lv_color_t selectColor = LV_COLOR_BLACK;

  for (unsigned int x = 0; x < FieldWidth; x++) {
    for (unsigned int y = 0; y < FieldHeight; y++) {

      switch (Field[y * FieldWidth + x]) {
        case BODY:
          selectColor = LV_COLOR_YELLOW;
          break;
        case SOLID:
          selectColor = LV_COLOR_WHITE;
          break;
        case FOOD:
          selectColor = LV_COLOR_GREEN;
          break;
        default:
          selectColor = LV_COLOR_BLACK;
          break;
      }
      for (unsigned int ti = 0; ti < TileBufferSize; ti++)
        TileBuffer[ti] = selectColor;

      lv_area_t area;

      area.x1 = x * TileSize + FieldOffsetHorizontal;
      area.y1 = y * TileSize + FieldOffsetVertical;
      area.x2 = area.x1 + TileSize - 1;
      area.y2 = area.y1 + TileSize - 1;
      lvgl.FlushDisplay(&area, TileBuffer);
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
      vTaskDelay(1); // Hack to give give the system time to refresh
    }
  }
}

void Adder::Refresh() {
  updateDisplay();
}

void Adder::updateSingleTile(unsigned int FieldPosX, unsigned int FieldPosY, lv_color_t Color) {
  for (unsigned int ti = 0; ti < TileBufferSize; ti++)
    TileBuffer[ti] = Color;

  lv_area_t area;
  area.x1 = FieldPosX * TileSize + FieldOffsetHorizontal;
  area.y1 = FieldPosY * TileSize + FieldOffsetVertical;
  area.x2 = area.x1 + TileSize - 1;
  area.y2 = area.y1 + TileSize - 1;
  lvgl.FlushDisplay(&area, TileBuffer);
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  vTaskDelay(1); // Hack to give give the system time to refresh
}

void Adder::updateDisplay() {
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  updatePosition();
  if (!AppReady) {
    FullReDraw();
    createFood();
    updateSingleTile(AdderBody.back() % FieldWidth, AdderBody.back() / FieldWidth, LV_COLOR_BLACK);
    updateScore(0);
    AppReady = true;
  } else {
    updateSingleTile(AdderBody.front() % FieldWidth, AdderBody.front() / FieldWidth, LV_COLOR_YELLOW);
    updateSingleTile(AdderBody.back() % FieldWidth, AdderBody.back() / FieldWidth, LV_COLOR_BLACK);
  }
}
