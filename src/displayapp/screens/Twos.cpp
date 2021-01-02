#include "Twos.h"
#include <lvgl/lvgl.h>
#include <string>
#include <charconv>
#include <array>
#include <vector>
#include <utility>

using namespace Pinetime::Applications::Screens;

extern lv_font_t jetbrains_mono_bold_20;

Twos::Twos(Pinetime::Applications::DisplayApp *app) : Screen(app) {

  // create styles to apply to different valued tiles
  static lv_style_t style_cell1;
  lv_style_copy(&style_cell1, &lv_style_plain);
  style_cell1.body.border.width = 1;
  style_cell1.text.font = &jetbrains_mono_bold_20;
  style_cell1.body.padding.top = 16;
  style_cell1.body.padding.bottom = 16;
  style_cell1.body.main_color = LV_COLOR_MAKE(214, 197, 165);
  style_cell1.body.grad_color = LV_COLOR_MAKE(214, 197, 165);
  style_cell1.text.color = LV_COLOR_BLACK;  

  static lv_style_t style_cell2;
  lv_style_copy(&style_cell2, &style_cell1);
  style_cell2.body.main_color = LV_COLOR_MAKE(209, 146, 92);
  style_cell2.body.grad_color = LV_COLOR_MAKE(209, 146, 92);
  style_cell2.text.color = LV_COLOR_WHITE;

  static lv_style_t style_cell3;
  lv_style_copy(&style_cell3, &style_cell2);
  style_cell3.body.main_color = LV_COLOR_MAKE(246, 94, 59);
  style_cell3.body.grad_color = LV_COLOR_MAKE(246, 94, 59);

  static lv_style_t style_cell4;
  lv_style_copy(&style_cell4, &style_cell3);
  style_cell4.body.main_color = LV_COLOR_MAKE(212, 170, 28);
  style_cell4.body.grad_color = LV_COLOR_MAKE(212, 170, 28);

  // format grid display
  gridDisplay = lv_table_create(lv_scr_act(), nullptr);
  lv_table_set_style(gridDisplay, LV_TABLE_STYLE_CELL1, &style_cell1);
  lv_table_set_style(gridDisplay, LV_TABLE_STYLE_CELL2, &style_cell2);
  lv_table_set_style(gridDisplay, LV_TABLE_STYLE_CELL3, &style_cell3);
  lv_table_set_style(gridDisplay, LV_TABLE_STYLE_CELL4, &style_cell4);
  lv_table_set_col_cnt(gridDisplay, 4);
  lv_table_set_row_cnt(gridDisplay, 4);
  lv_table_set_col_width(gridDisplay, 0, LV_HOR_RES/4);
  lv_table_set_col_width(gridDisplay, 1, LV_HOR_RES/4);
  lv_table_set_col_width(gridDisplay, 2, LV_HOR_RES/4);
  lv_table_set_col_width(gridDisplay, 3, LV_HOR_RES/4);
  lv_obj_align(gridDisplay, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  // initialize grid
  for(int row = 0; row < 4; row++) {
    for(int col = 0; col < 4; col++) {
      grid[row][col].value = 0;
      lv_table_set_cell_type(gridDisplay, row, col, 2);
      lv_table_set_cell_align(gridDisplay, row, col, LV_LABEL_ALIGN_CENTER);
    }
  }
  placeNewTile();
  placeNewTile();

  // format score text
  scoreText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_width(scoreText, LV_HOR_RES);
  lv_label_set_align(scoreText, LV_ALIGN_IN_LEFT_MID);
  lv_obj_align(scoreText, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_label_set_text(scoreText, ("Score: " + std::to_string(score)).c_str());
}

Twos::~Twos() {
  lv_obj_clean(lv_scr_act());
}

bool Twos::Refresh() {
  return running;
}

bool Twos::OnButtonPushed() {
  running = false;
  return true;
}

bool Twos::placeNewTile() {
  std::vector< std::pair <int,int> > availableCells; 
  for(int row = 0; row < 4; row++) {
    for(int col = 0; col < 4; col++) {
      if(!grid[row][col].value) {
        availableCells.push_back(std::make_pair(row, col));
      }
    }
  }

  if (availableCells.size() == 0) {
    return false; // game lost
  }
  
  auto it = availableCells.cbegin();
  int random = rand() % availableCells.size();
  std::advance(it, random);
  std::pair <int,int> newCell = *it;

  if ((rand() % 100) < 90) grid[newCell.first][newCell.second].value = 2;
  else grid[newCell.first][newCell.second].value = 4;
  updateGridDisplay(grid);
  return true;
}

bool Twos::tryMerge(Tile grid[][4], int &newRow, int &newCol, int oldRow, int oldCol) {
  if((grid[newRow][newCol].value == grid[oldRow][oldCol].value)) {
    if((newCol != oldCol) || (newRow != oldRow)) {
      if(!grid[newRow][newCol].merged) {
        unsigned int newVal = grid[oldRow][oldCol].value *= 2;
        grid[newRow][newCol].value = newVal;
        score += newVal;
        lv_label_set_text(scoreText, ("Score: " + std::to_string(score)).c_str());
        grid[oldRow][oldCol].value = 0;
        grid[newRow][newCol].merged = true;
        return true;
      }
    }
  }
  return false;
}

bool Twos::tryMove(Tile grid[][4], int newRow, int newCol, int oldRow, int oldCol) {
  if(((newCol >= 0) && (newCol != oldCol)) || ((newRow >= 0) && (newRow != oldRow))) {
    grid[newRow][newCol].value = grid[oldRow][oldCol].value;
    grid[oldRow][oldCol].value = 0;
    return true;
  }
  return false;
}

bool Twos::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  bool validMove;
  validMove = false;
  for(int row = 0; row < 4; row++) {
    for(int col = 0; col < 4; col++) {
      grid[row][col].merged = false; // reinitialize merge state
    }
  }
  switch(event) {
    case TouchEvents::SwipeLeft:
      for(int col = 1; col < 4; col++) { // ignore tiles already on far left
        for(int row = 0; row < 4; row++) {
          if(grid[row][col].value) {
            int newCol = -1;
            for(int potentialNewCol = col - 1; potentialNewCol >= 0; potentialNewCol--) {
              if(!grid[row][potentialNewCol].value) {
                newCol = potentialNewCol;
              }
              else { // blocked by another tile
                if(tryMerge(grid, row, potentialNewCol, row, col)) validMove = true;
                break;
              }
            }
            if(tryMove(grid, row, newCol, row, col)) validMove = true;
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      return true;
    case TouchEvents::SwipeRight:
      for(int col = 2; col >= 0; col--) { // ignore tiles already on far right
        for(int row = 0; row < 4; row++) {
          if(grid[row][col].value) {
            int newCol = -1;
            for(int potentialNewCol = col + 1; potentialNewCol < 4; potentialNewCol++) {
              if(!grid[row][potentialNewCol].value) {
                newCol = potentialNewCol;
              }
              else { // blocked by another tile
                if(tryMerge(grid, row, potentialNewCol, row, col)) validMove = true;
                break;
              }
            }
            if(tryMove(grid, row, newCol, row, col)) validMove = true;
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      return true;
    case TouchEvents::SwipeUp:
      for(int row = 1; row < 4; row++) { // ignore tiles already on top
        for(int col = 0; col < 4; col++) {
          if(grid[row][col].value) {
            int newRow = -1;
            for(int potentialNewRow = row - 1; potentialNewRow >= 0; potentialNewRow--) {
              if(!grid[potentialNewRow][col].value) {
                newRow = potentialNewRow;
              }
              else { // blocked by another tile
                if(tryMerge(grid, potentialNewRow, col, row, col)) validMove = true;
                break;
              }
            }
            if(tryMove(grid, newRow, col, row, col)) validMove = true;
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      return true;
    case TouchEvents::SwipeDown:
      for(int row = 2; row >=0; row--) { // ignore tiles already on bottom
        for(int col = 0; col < 4; col++) {
          if(grid[row][col].value) {
            int newRow = -1;
            for(int potentialNewRow = row + 1; potentialNewRow < 4; potentialNewRow++) {         
              if(!grid[potentialNewRow][col].value) {
                newRow = potentialNewRow;
              }
              else { // blocked by another tile
                if(tryMerge(grid, potentialNewRow, col, row, col)) validMove = true;
                break;
              }
            }
            if(tryMove(grid, newRow, col, row, col)) validMove = true;
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      return true;
    default:
      return false;
  }
  return false;
}

void Twos::updateGridDisplay(Tile grid[][4]) {
  for(int row = 0; row < 4; row++) {
    for(int col = 0; col < 4; col++) {
      if (grid[row][col].value) {
        lv_table_set_cell_value(gridDisplay, row, col, (std::to_string(grid[row][col].value)).c_str());
      }
      else {
        lv_table_set_cell_value(gridDisplay, row, col, "");
      }
      switch (grid[row][col].value) {
        case 0:
        case 2:
        case 4:
          lv_table_set_cell_type(gridDisplay, row, col, 1);
          break;
        case 8:
        case 16:
          lv_table_set_cell_type(gridDisplay, row, col, 2);
          break;
        case 32:
        case 64:
          lv_table_set_cell_type(gridDisplay, row, col, 3);
          break;
        default:
          lv_table_set_cell_type(gridDisplay, row, col, 4);
          break;
      }
    }
  }
}