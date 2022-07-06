#include "displayapp/screens/Twos.h"
#include <cstdio>
#include <cstdlib>
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

Twos::Twos(Pinetime::Applications::DisplayApp* app) : Screen(app) {

  // create styles to apply to different valued tiles
  lv_style_init(&style_cell1);
  lv_style_init(&style_cell2);
  lv_style_init(&style_cell3);
  lv_style_init(&style_cell4);
  lv_style_init(&style_cell5);

  lv_style_set_border_color(&style_cell1, LV_STATE_DEFAULT, lv_color_hex(0xbbada0));
  lv_style_set_border_width(&style_cell1, LV_STATE_DEFAULT, 3);
  lv_style_set_bg_opa(&style_cell1, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_cell1, LV_STATE_DEFAULT, lv_color_hex(0xcdc0b4));
  lv_style_set_pad_top(&style_cell1, LV_STATE_DEFAULT, 29);
  lv_style_set_text_color(&style_cell1, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  lv_style_set_border_color(&style_cell2, LV_STATE_DEFAULT, lv_color_hex(0xbbada0));
  lv_style_set_border_width(&style_cell2, LV_STATE_DEFAULT, 3);
  lv_style_set_bg_opa(&style_cell2, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_cell2, LV_STATE_DEFAULT, lv_color_hex(0xefdfc6));
  lv_style_set_pad_top(&style_cell2, LV_STATE_DEFAULT, 29);
  lv_style_set_text_color(&style_cell2, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  lv_style_set_border_color(&style_cell3, LV_STATE_DEFAULT, lv_color_hex(0xbbada0));
  lv_style_set_border_width(&style_cell3, LV_STATE_DEFAULT, 3);
  lv_style_set_bg_opa(&style_cell3, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_cell3, LV_STATE_DEFAULT, lv_color_hex(0xef9263));
  lv_style_set_pad_top(&style_cell3, LV_STATE_DEFAULT, 29);

  lv_style_set_border_color(&style_cell4, LV_STATE_DEFAULT, lv_color_hex(0xbbada0));
  lv_style_set_border_width(&style_cell4, LV_STATE_DEFAULT, 3);
  lv_style_set_bg_opa(&style_cell4, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_cell4, LV_STATE_DEFAULT, lv_color_hex(0xf76142));
  lv_style_set_pad_top(&style_cell4, LV_STATE_DEFAULT, 29);

  lv_style_set_border_color(&style_cell5, LV_STATE_DEFAULT, lv_color_hex(0xbbada0));
  lv_style_set_border_width(&style_cell5, LV_STATE_DEFAULT, 3);
  lv_style_set_bg_opa(&style_cell5, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_bg_color(&style_cell5, LV_STATE_DEFAULT, lv_color_hex(0x007dc5));
  lv_style_set_pad_top(&style_cell5, LV_STATE_DEFAULT, 29);

  // format grid display

  gridDisplay = lv_table_create(lv_scr_act(), nullptr);
  lv_obj_add_style(gridDisplay, LV_TABLE_PART_CELL1, &style_cell1);
  lv_obj_add_style(gridDisplay, LV_TABLE_PART_CELL2, &style_cell2);
  lv_obj_add_style(gridDisplay, LV_TABLE_PART_CELL3, &style_cell3);
  lv_obj_add_style(gridDisplay, LV_TABLE_PART_CELL4, &style_cell4);
  lv_obj_add_style(gridDisplay, LV_TABLE_PART_CELL4 + 1, &style_cell5);
  lv_table_set_col_cnt(gridDisplay, nCols);
  lv_table_set_row_cnt(gridDisplay, nRows);
  for (int col = 0; col < nCols; col++) {
    static constexpr int colWidth = LV_HOR_RES_MAX / nCols;
    lv_table_set_col_width(gridDisplay, col, colWidth);
    for (int row = 0; row < nRows; row++) {
      grid[row][col].value = 0;
      lv_table_set_cell_type(gridDisplay, row, col, 1);
      lv_table_set_cell_align(gridDisplay, row, col, LV_LABEL_ALIGN_CENTER);
    }
  }
  // Move one pixel down to remove a gap
  lv_obj_align(gridDisplay, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 1);

  lv_obj_clean_style_list(gridDisplay, LV_TABLE_PART_BG);

  placeNewTile();
  placeNewTile();

  // format score text
  scoreText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_width(scoreText, LV_HOR_RES);
  lv_label_set_align(scoreText, LV_ALIGN_IN_LEFT_MID);
  lv_obj_align(scoreText, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_label_set_recolor(scoreText, true);
  lv_label_set_text_fmt(scoreText, "Score #FFFF00 %i#", score);
}

Twos::~Twos() {
  lv_style_reset(&style_cell1);
  lv_style_reset(&style_cell2);
  lv_style_reset(&style_cell3);
  lv_style_reset(&style_cell4);
  lv_style_reset(&style_cell5);
  lv_obj_clean(lv_scr_act());
}

bool Twos::placeNewTile() {
  unsigned int emptyCells[nCells];
  unsigned int nEmpty = 0;
  for (unsigned int i = 0; i < nCells; i++) {
    const unsigned int row = i / nCols;
    const unsigned int col = i % nCols;
    if (grid[row][col].value == 0) {
      emptyCells[nEmpty] = i;
      nEmpty++;
    }
  }

  if (nEmpty == 0) {
    return false; // game lost
  }

  int random = rand() % nEmpty;

  if ((rand() % 100) < 90) {
    grid[emptyCells[random] / nCols][emptyCells[random] % nCols].value = 2;
  } else {
    grid[emptyCells[random] / nCols][emptyCells[random] % nCols].value = 4;
  }
  updateGridDisplay();
  return true;
}

bool Twos::tryMerge(int newRow, int newCol, int oldRow, int oldCol) {
  if (grid[newRow][newCol].value == grid[oldRow][oldCol].value) {
    if ((newCol != oldCol) || (newRow != oldRow)) {
      if (!grid[newRow][newCol].merged) {
        grid[newRow][newCol].value *= 2;
        score += grid[newRow][newCol].value;
        lv_label_set_text_fmt(scoreText, "Score #FFFF00 %i#", score);
        grid[oldRow][oldCol].value = 0;
        grid[newRow][newCol].merged = true;
        return true;
      }
    }
  }
  return false;
}

bool Twos::tryMove(int newRow, int newCol, int oldRow, int oldCol) {
  if (((newCol >= 0) && (newCol != oldCol)) || ((newRow >= 0) && (newRow != oldRow))) {
    grid[newRow][newCol].value = grid[oldRow][oldCol].value;
    grid[oldRow][oldCol].value = 0;
    return true;
  }
  return false;
}

bool Twos::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  bool validMove = false;
  for (unsigned int i = 0; i < nCells; i++) {
    const unsigned int row = i / nCols;
    const unsigned int col = i % nCols;
    grid[row][col].merged = false; // reinitialize merge state
  }
  switch (event) {
    case TouchEvents::SwipeLeft:
      for (int col = 1; col < nCols; col++) { // ignore tiles already on far left
        for (int row = 0; row < nRows; row++) {
          if (grid[row][col].value > 0) {
            int newCol = -1;
            for (int potentialNewCol = col - 1; potentialNewCol >= 0; potentialNewCol--) {
              if (grid[row][potentialNewCol].value == 0) {
                newCol = potentialNewCol;
              } else { // blocked by another tile
                if (tryMerge(row, potentialNewCol, row, col)) {
                  validMove = true;
                }
                break;
              }
            }
            if (tryMove(row, newCol, row, col)) {
              validMove = true;
            }
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      return true;
    case TouchEvents::SwipeRight:
      for (int col = nCols - 2; col >= 0; col--) { // ignore tiles already on far right
        for (int row = 0; row < nRows; row++) {
          if (grid[row][col].value > 0) {
            int newCol = -1;
            for (int potentialNewCol = col + 1; potentialNewCol < nCols; potentialNewCol++) {
              if (grid[row][potentialNewCol].value == 0) {
                newCol = potentialNewCol;
              } else { // blocked by another tile
                if (tryMerge(row, potentialNewCol, row, col)) {
                  validMove = true;
                }
                break;
              }
            }
            if (tryMove(row, newCol, row, col)) {
              validMove = true;
            }
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      return true;
    case TouchEvents::SwipeUp:
      for (int row = 1; row < nRows; row++) { // ignore tiles already on top
        for (int col = 0; col < nCols; col++) {
          if (grid[row][col].value > 0) {
            int newRow = -1;
            for (int potentialNewRow = row - 1; potentialNewRow >= 0; potentialNewRow--) {
              if (grid[potentialNewRow][col].value == 0) {
                newRow = potentialNewRow;
              } else { // blocked by another tile
                if (tryMerge(potentialNewRow, col, row, col)) {
                  validMove = true;
                }
                break;
              }
            }
            if (tryMove(newRow, col, row, col)) {
              validMove = true;
            }
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      return true;
    case TouchEvents::SwipeDown:
      for (int row = nRows - 2; row >= 0; row--) { // ignore tiles already on bottom
        for (int col = 0; col < nCols; col++) {
          if (grid[row][col].value > 0) {
            int newRow = -1;
            for (int potentialNewRow = row + 1; potentialNewRow < nRows; potentialNewRow++) {
              if (grid[potentialNewRow][col].value == 0) {
                newRow = potentialNewRow;
              } else { // blocked by another tile
                if (tryMerge(potentialNewRow, col, row, col)) {
                  validMove = true;
                }
                break;
              }
            }
            if (tryMove(newRow, col, row, col)) {
              validMove = true;
            }
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

void Twos::updateGridDisplay() {
  for (unsigned int i = 0; i < nCells; i++) {
    const unsigned int row = i / nCols;
    const unsigned int col = i % nCols;
    if (grid[row][col].value > 0) {
      char buffer[7];
      sprintf(buffer, "%d", grid[row][col].value);
      lv_table_set_cell_value(gridDisplay, row, col, buffer);
    } else {
      lv_table_set_cell_value(gridDisplay, row, col, "");
    }
    switch (grid[row][col].value) {
      case 0:
        lv_table_set_cell_type(gridDisplay, row, col, 1);
        break;
      case 2:
      case 4:
        lv_table_set_cell_type(gridDisplay, row, col, 2);
        break;
      case 8:
      case 16:
        lv_table_set_cell_type(gridDisplay, row, col, 3);
        break;
      case 32:
      case 64:
        lv_table_set_cell_type(gridDisplay, row, col, 4);
        break;
      default:
        lv_table_set_cell_type(gridDisplay, row, col, 5);
        break;
    }
  }
}
