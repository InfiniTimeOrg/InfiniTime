#include "displayapp/screens/Twos.h"
#include <cstdio>
#include <cstdlib>
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

bool TwosGrid::PlaceNewTile() {
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
  return true;
}

void TwosGrid::ResetMergeState() {
  for (auto& gridRow : grid) {
    for (TwosTile& tile : gridRow) {
      tile.merged = false;
    }
  }
}

bool TwosGrid::TryMerge(int newRow, int newCol, int oldRow, int oldCol) {
  if (grid[newRow][newCol].value == grid[oldRow][oldCol].value) {
    if ((newCol != oldCol) || (newRow != oldRow)) {
      if (!grid[newRow][newCol].merged) {
        grid[newRow][newCol].value *= 2;
        score += grid[newRow][newCol].value;
        grid[oldRow][oldCol].value = 0;
        grid[newRow][newCol].merged = true;
        return true;
      }
    }
  }
  return false;
}

bool TwosGrid::TryMove(int newRow, int newCol, int oldRow, int oldCol) {
  if ((newRow == oldRow) && (newCol == oldCol)) {
    return false;
  }
  grid[newRow][newCol].value = grid[oldRow][oldCol].value;
  grid[oldRow][oldCol].value = 0;
  return true;
}

bool TwosGrid::Slide(int rowStep, int colStep) {
  ResetMergeState();

  bool validMove = false;
  // Tiles closest to the edge they are sliding towards have to be resolved
  // first, so that the ones behind them can use the space they free up.
  for (int i = 0; i < nRows; i++) {
    const int row = rowStep > 0 ? nRows - 1 - i : i;
    for (int j = 0; j < nCols; j++) {
      const int col = colStep > 0 ? nCols - 1 - j : j;
      if (grid[row][col].value == 0) {
        continue;
      }

      // Look for the furthest empty cell in this direction, stopping at the
      // first tile in the way and merging with it if possible.
      int newRow = row;
      int newCol = col;
      for (int r = row + rowStep, c = col + colStep; (r >= 0) && (r < nRows) && (c >= 0) && (c < nCols); r += rowStep, c += colStep) {
        if (grid[r][c].value == 0) {
          newRow = r;
          newCol = c;
        } else { // blocked by another tile
          if (TryMerge(r, c, row, col)) {
            validMove = true;
          }
          break;
        }
      }

      if (TryMove(newRow, newCol, row, col)) {
        validMove = true;
      }
    }
  }

  return validMove;
}

Twos::Twos() {

  struct colorPair {
    lv_color_t bg;
    lv_color_t fg;
  };

  static constexpr colorPair colors[nColors] = {
    {LV_COLOR_MAKE(0xcd, 0xc0, 0xb4), LV_COLOR_BLACK},
    {LV_COLOR_MAKE(0xef, 0xdf, 0xc6), LV_COLOR_BLACK},
    {LV_COLOR_MAKE(0xef, 0x92, 0x63), LV_COLOR_WHITE},
    {LV_COLOR_MAKE(0xf7, 0x61, 0x42), LV_COLOR_WHITE},
    {LV_COLOR_MAKE(0x00, 0x7d, 0xc5), LV_COLOR_WHITE},
  };

  gridDisplay = lv_table_create(lv_scr_act(), nullptr);

  for (size_t i = 0; i < nColors; i++) {
    lv_style_init(&cellStyles[i]);

    lv_style_set_border_color(&cellStyles[i], LV_STATE_DEFAULT, lv_color_hex(0xbbada0));
    lv_style_set_border_width(&cellStyles[i], LV_STATE_DEFAULT, 3);
    lv_style_set_bg_opa(&cellStyles[i], LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&cellStyles[i], LV_STATE_DEFAULT, colors[i].bg);
    lv_style_set_pad_top(&cellStyles[i], LV_STATE_DEFAULT, 29);
    lv_style_set_text_color(&cellStyles[i], LV_STATE_DEFAULT, colors[i].fg);

    lv_obj_add_style(gridDisplay, LV_TABLE_PART_CELL1 + i, &cellStyles[i]);
  }

  lv_table_set_col_cnt(gridDisplay, TwosGrid::nCols);
  lv_table_set_row_cnt(gridDisplay, TwosGrid::nRows);
  for (int col = 0; col < TwosGrid::nCols; col++) {
    static constexpr int colWidth = LV_HOR_RES_MAX / TwosGrid::nCols;
    lv_table_set_col_width(gridDisplay, col, colWidth);
    for (int row = 0; row < TwosGrid::nRows; row++) {
      lv_table_set_cell_type(gridDisplay, row, col, 1);
      lv_table_set_cell_align(gridDisplay, row, col, LV_LABEL_ALIGN_CENTER);
    }
  }
  // Move one pixel down to remove a gap
  lv_obj_align(gridDisplay, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 1);

  lv_obj_clean_style_list(gridDisplay, LV_TABLE_PART_BG);

  grid.PlaceNewTile();
  grid.PlaceNewTile();
  UpdateGridDisplay();

  // format score text
  scoreText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_width(scoreText, LV_HOR_RES);
  lv_label_set_align(scoreText, LV_ALIGN_IN_LEFT_MID);
  lv_obj_align(scoreText, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_label_set_recolor(scoreText, true);
  UpdateScoreDisplay();
}

Twos::~Twos() {
  for (lv_style_t cellStyle : cellStyles) {
    lv_style_reset(&cellStyle);
  }
  lv_obj_clean(lv_scr_act());
}

bool Twos::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  int rowStep = 0;
  int colStep = 0;
  switch (event) {
    case TouchEvents::SwipeLeft:
      colStep = -1;
      break;
    case TouchEvents::SwipeRight:
      colStep = 1;
      break;
    case TouchEvents::SwipeUp:
      rowStep = -1;
      break;
    case TouchEvents::SwipeDown:
      rowStep = 1;
      break;
    default:
      return false;
  }

  if (grid.Slide(rowStep, colStep)) {
    grid.PlaceNewTile();
    UpdateGridDisplay();
    UpdateScoreDisplay();
  }
  return true;
}

void Twos::UpdateScoreDisplay() {
  lv_label_set_text_fmt(scoreText, "Score #FFFF00 %i#", grid.GetScore());
}

void Twos::UpdateGridDisplay() {
  for (unsigned int i = 0; i < TwosGrid::nCells; i++) {
    const unsigned int row = i / TwosGrid::nCols;
    const unsigned int col = i % TwosGrid::nCols;
    const unsigned int value = grid.GetTileValue(row, col);
    if (value > 0) {
      // Large enough for any unsigned int, so the value can never be truncated
      char buffer[11];
      snprintf(buffer, sizeof(buffer), "%u", value);
      lv_table_set_cell_value(gridDisplay, row, col, buffer);
    } else {
      lv_table_set_cell_value(gridDisplay, row, col, "");
    }
    switch (value) {
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
