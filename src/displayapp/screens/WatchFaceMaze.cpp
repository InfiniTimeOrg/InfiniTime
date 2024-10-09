#include "displayapp/screens/WatchFaceMaze.h"

using namespace Pinetime::Applications::Screens;


Maze::Maze() {
  std::fill_n(mazemap, FLATSIZE, 0);
}

uint8_t Maze::get(int x, int y) {
  if (x<0||x>WIDTH||y<0||y>HEIGHT) {return 0;}
  return get((y * WIDTH) + x);
}
uint8_t Maze::get(int index) {
  if (index < 0 || index/2 >= FLATSIZE) {return 0;}
  // odd means right (low) nibble, even means left (high) nibble
  if (index & 0b1) return mazemap[index/2] & 0b00001111;
  else             return mazemap[index/2] >> 4;
}

void Maze::set(int x, int y, uint8_t value) {
  if (x<0||x>WIDTH||y<0||y>HEIGHT) {return;}
  set(y * WIDTH + x, value);
}
void Maze::set(int index, uint8_t value) {
  if (index < 0 || index/2 >= FLATSIZE) {return;}
  // odd means right (low) nibble, even means left (high) nibble
  if (index & 0b1) mazemap[index/2] = (mazemap[index/2] & 0b11110000) | (value & 0b00001111);
  else             mazemap[index/2] = (mazemap[index/2] & 0b00001111) | (value << 4);
}




WatchFaceMaze::WatchFaceMaze(Pinetime::Components::LittleVgl& lvgl,
                             Controllers::DateTime& dateTimeController)
  : dateTimeController {dateTimeController},
    lvgl {lvgl},
    maze {Maze()} {

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  //Refresh();
}

WatchFaceMaze::~WatchFaceMaze() {
  lv_obj_clean(lv_scr_act());
  lv_task_del(taskRefresh);
}

void WatchFaceMaze::GenerateMaze() {
  for (int i = 0; i < Maze::FLATSIZE*2; i++) {
    maze.set(i, rand() & 0b11);
    //maze.set(i, 0b11);
  }
}

void WatchFaceMaze::Refresh() {
  currentDateTime = dateTimeController.CurrentDateTime();
  if (currentDateTime.IsUpdated()) {
    GenerateMaze();
    //ProcessCorners();
    DrawMaze();
  }
}

void WatchFaceMaze::DrawMaze() {
  lv_area_t area;
  lv_color_t *curbuf = buf2;
  // print horizontal lines
  area.x1 = 0;
  area.x2 = 239;
  for (int y = 0; y < Maze::HEIGHT; y++) {
    curbuf = (curbuf==buf1) ? buf2 : buf1;  // switch buffer
    for (int x = 0; x < Maze::WIDTH; x++) {
      if (maze.get(x, y) & 0b01)  {std::fill_n(&curbuf[x*Maze::TILESIZE], Maze::TILESIZE, LV_COLOR_WHITE);}
      else                        {std::fill_n(&curbuf[x*Maze::TILESIZE], Maze::TILESIZE, LV_COLOR_BLACK);}
    }
    std::copy_n(curbuf, 240, &curbuf[240]);
    area.y1 = Maze::TILESIZE * y;
    area.y2 = Maze::TILESIZE * y + 1;
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, curbuf);
  }
  // print vertical lines
  area.y1 = 0;
  area.y2 = 239;
  for (int x = 0; x < Maze::WIDTH; x++) {
    curbuf = (curbuf==buf1) ? buf2 : buf1;  // switch buffer
    for (int y = 0; y < Maze::HEIGHT; y++) {
      uint8_t curblock = maze.get(x,y);
      // handle corners: if any of the touching lines are present, add corner. else leave it black
      if (curblock & 0b11 || maze.get(x-1,y) & 0b01 | maze.get(x,y-1) & 0b10)
            {std::fill_n(&curbuf[y*Maze::TILESIZE*2], 4, LV_COLOR_WHITE);}
      else  {std::fill_n(&curbuf[y*Maze::TILESIZE*2], 4, LV_COLOR_BLACK);}

      if (curblock & 0b010) {std::fill_n(&curbuf[y*Maze::TILESIZE*2+4], Maze::TILESIZE*2-4, LV_COLOR_WHITE);}
      else                  {std::fill_n(&curbuf[y*Maze::TILESIZE*2+4], Maze::TILESIZE*2-4, LV_COLOR_BLACK);}
    }
    area.x1 = Maze::TILESIZE * x;
    area.x2 = Maze::TILESIZE * x + 1;
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&area, curbuf);

  }
}