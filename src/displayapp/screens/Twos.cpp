#include "Twos.h"
#include <lvgl/lvgl.h>
#include <string>
#include <charconv>
#include <array>
#include <vector>
#include <utility>

using namespace Pinetime::Applications::Screens;


namespace {
  const lv_color_t TWOS_COLOR_0 = lv_color_hex(0xcdc0b4);
  const lv_color_t TWOS_COLOR_2_4 = lv_color_hex(0xefdfc6);
  const lv_color_t TWOS_COLOR_8_16 = lv_color_hex(0xef9263);
  const lv_color_t TWOS_COLOR_32_64 = lv_color_hex(0xf76142);
  const lv_color_t TWOS_COLOR_128_PLUS = lv_color_hex(0x007dc5);
  const lv_color_t TWOS_BORDER_DEFAULT = lv_color_hex(0xbbada0);
  const lv_color_t TWOS_BORDER_NEW = lv_color_darken(lv_color_hex(0xbbada0), LV_OPA_50);
}

Twos::Twos(Pinetime::Applications::DisplayApp* app) : Screen(app) {

  // create styles to apply to different valued tiles
  lv_style_init(&style_cell_default);

  lv_style_set_border_color(&style_cell_default, TWOS_BORDER_DEFAULT);
  lv_style_set_border_width(&style_cell_default, 3);
  lv_style_set_bg_opa(&style_cell_default, LV_OPA_COVER);
  lv_style_set_bg_color(&style_cell_default, TWOS_COLOR_0);
  lv_style_set_pad_top(&style_cell_default, 25);
  lv_style_set_text_color(&style_cell_default, lv_color_black());
  lv_style_set_radius(&style_cell_default, 0);
  lv_style_set_pad_all(&style_cell_default, 0);

  // format grid display

  twosContainer = lv_obj_create(lv_scr_act());
  lv_obj_set_size(twosContainer, LV_HOR_RES, LV_VER_RES-40);
  lv_obj_align(twosContainer, LV_ALIGN_BOTTOM_MID, 0, 0);
  int cell_w = LV_HOR_RES/4;
  int cell_h = (LV_VER_RES-40)/4;
  lv_obj_t *align_to = twosContainer;
  lv_align_t align_by = LV_ALIGN_TOP_LEFT;
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      Tile *curTile = &grid[row][col];
      
      curTile->box = lv_obj_create(twosContainer);
      lv_obj_add_style(curTile->box, &style_cell_default, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_size(curTile->box, cell_w, cell_h);
      
      curTile->lbl = lv_label_create(curTile->box);
      lv_obj_clear_flag(curTile->lbl, LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_center(curTile->lbl);
      lv_label_set_text(curTile->lbl, "");
      lv_label_set_long_mode(curTile->lbl, LV_LABEL_LONG_CLIP);
      
      lv_obj_align_to(curTile->box, align_to, align_by, 0, 0);
      if(col != 3){
        align_to = curTile->box;
        align_by = LV_ALIGN_OUT_RIGHT_MID;
      } else{
        align_to = grid[row][0].box;
        align_by = LV_ALIGN_OUT_BOTTOM_MID;
      }
      
    }
  }
  placeNewTile();
  placeNewTile();

  // format score text
  scoreText = lv_label_create(lv_scr_act());
  lv_obj_set_width(scoreText, LV_HOR_RES);
  lv_obj_set_style_text_align(scoreText, LV_ALIGN_LEFT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(scoreText, LV_ALIGN_TOP_LEFT, -10, 10);
  lv_label_set_recolor(scoreText, true);
  lv_label_set_text_fmt(scoreText, "Score #FFFF00 %i#", score);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");
}

Twos::~Twos() {
  lv_style_reset(&style_cell_default);
  lv_obj_clean(lv_scr_act());
}

bool Twos::placeNewTile() {
  std::vector<std::pair<int, int>> availableCells;
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      if(grid[row][col].isNew){
        grid[row][col].isNew = false;
        grid[row][col].changed = true;
      }
      if (!grid[row][col].value) {
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
  std::pair<int, int> newCell = *it;
  Tile *curTile = &grid[newCell.first][newCell.second];

  if ((rand() % 100) < 90) {
    curTile->value = 2;
  } else {
    curTile->value = 4;
  }
  curTile->changed = true;
  curTile->isNew = true;
  updateGridDisplay(grid);
  return true;
}

void Twos::updateTileColor(Tile *tile){
  lv_color_t color;
  switch (tile->value) {
    case 0:
      color = TWOS_COLOR_0;
      break;
    case 2:
    case 4:
      color = TWOS_COLOR_2_4;
      break;
    case 8:
    case 16:
      color = TWOS_COLOR_8_16;
      break;
    case 32:
    case 64:
      color = TWOS_COLOR_32_64;
      break;
    default:
      color = TWOS_COLOR_128_PLUS;
      break;
  }
  lv_obj_set_style_bg_color(tile->box, color, LV_PART_MAIN | LV_STATE_DEFAULT);
  
  lv_color_t borderColor = tile->isNew ? TWOS_BORDER_NEW : TWOS_BORDER_DEFAULT;
  lv_obj_set_style_border_color(tile->box, borderColor, LV_PART_MAIN | LV_STATE_DEFAULT);
}

bool Twos::tryMerge(Tile grid[][4], int& newRow, int& newCol, int oldRow, int oldCol) {
  if (grid[newRow][newCol].value == grid[oldRow][oldCol].value) {
    if ((newCol != oldCol) || (newRow != oldRow)) {
      if (!grid[newRow][newCol].merged) {
        unsigned int newVal = grid[oldRow][oldCol].value *= 2;
        grid[newRow][newCol].value = newVal;
        score += newVal;
        lv_label_set_text_fmt(scoreText, "Score #FFFF00 %i#", score);
        grid[oldRow][oldCol].value = 0;
        grid[newRow][newCol].merged = true;
        
        grid[oldRow][oldCol].changed = true;
        grid[newRow][newCol].changed = true;
        
        return true;
      }
    }
  }
  return false;
}

bool Twos::tryMove(Tile grid[][4], int newRow, int newCol, int oldRow, int oldCol) {
  if (((newCol >= 0) && (newCol != oldCol)) || ((newRow >= 0) && (newRow != oldRow))) {
    grid[newRow][newCol].value = grid[oldRow][oldCol].value;
    grid[oldRow][oldCol].value = 0;
    
    grid[oldRow][oldCol].changed = true;
    grid[newRow][newCol].changed = true;
    
    return true;
  }
  return false;
}

bool Twos::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  bool validMove = false;
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      grid[row][col].merged = false; // reinitialize merge state
    }
  }
  switch (event) {
    case TouchEvents::SwipeLeft:
      for (int col = 1; col < 4; col++) { // ignore tiles already on far left
        for (int row = 0; row < 4; row++) {
          if (grid[row][col].value) {
            int newCol = -1;
            for (int potentialNewCol = col - 1; potentialNewCol >= 0; potentialNewCol--) {
              if (!grid[row][potentialNewCol].value) {
                newCol = potentialNewCol;
              } else { // blocked by another tile
                if (tryMerge(grid, row, potentialNewCol, row, col))
                  validMove = true;
                break;
              }
            }
            if (tryMove(grid, row, newCol, row, col))
              validMove = true;
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      break;
    case TouchEvents::SwipeRight:
      for (int col = 2; col >= 0; col--) { // ignore tiles already on far right
        for (int row = 0; row < 4; row++) {
          if (grid[row][col].value) {
            int newCol = -1;
            for (int potentialNewCol = col + 1; potentialNewCol < 4; potentialNewCol++) {
              if (!grid[row][potentialNewCol].value) {
                newCol = potentialNewCol;
              } else { // blocked by another tile
                if (tryMerge(grid, row, potentialNewCol, row, col))
                  validMove = true;
                break;
              }
            }
            if (tryMove(grid, row, newCol, row, col))
              validMove = true;
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      break;
    case TouchEvents::SwipeUp:
      for (int row = 1; row < 4; row++) { // ignore tiles already on top
        for (int col = 0; col < 4; col++) {
          if (grid[row][col].value) {
            int newRow = -1;
            for (int potentialNewRow = row - 1; potentialNewRow >= 0; potentialNewRow--) {
              if (!grid[potentialNewRow][col].value) {
                newRow = potentialNewRow;
              } else { // blocked by another tile
                if (tryMerge(grid, potentialNewRow, col, row, col))
                  validMove = true;
                break;
              }
            }
            if (tryMove(grid, newRow, col, row, col))
              validMove = true;
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      break;
    case TouchEvents::SwipeDown:
      for (int row = 2; row >= 0; row--) { // ignore tiles already on bottom
        for (int col = 0; col < 4; col++) {
          if (grid[row][col].value) {
            int newRow = -1;
            for (int potentialNewRow = row + 1; potentialNewRow < 4; potentialNewRow++) {
              if (!grid[potentialNewRow][col].value) {
                newRow = potentialNewRow;
              } else { // blocked by another tile
                if (tryMerge(grid, potentialNewRow, col, row, col))
                  validMove = true;
                break;
              }
            }
            if (tryMove(grid, newRow, col, row, col))
              validMove = true;
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      break;
    default:
      validMove = false;
      break;
  }
  return validMove;
}

void Twos::updateGridDisplay(Tile grid[][4]) {
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      Tile *curTile = &grid[row][col];
      if (curTile->changed){
        if (curTile->value) {
          lv_label_set_text(curTile->lbl, std::to_string(curTile->value).c_str());
        } else {
          lv_label_set_text(curTile->lbl, "");
        }
        updateTileColor(curTile);
      }
    }
  }
}
