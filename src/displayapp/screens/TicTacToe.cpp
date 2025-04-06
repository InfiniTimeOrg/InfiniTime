#include "displayapp/screens/TicTacToe.h"
#include <nrf_log.h>
using namespace Pinetime::Applications::Screens;

TicTacToe::TicTacToe() {
  lv_style_init(&cellStyle);
  lv_style_set_border_color(&cellStyle, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_style_set_border_width(&cellStyle, LV_STATE_DEFAULT, 4);
  lv_style_set_bg_color(&cellStyle, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_pad_top(&cellStyle, LV_STATE_DEFAULT, 28);
  lv_style_set_pad_bottom(&cellStyle, LV_STATE_DEFAULT, 28);
  lv_style_set_border_side(&cellStyle, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);

  playField = lv_table_create(lv_scr_act(), NULL);
  lv_table_set_col_cnt(playField, 3);
  lv_table_set_row_cnt(playField, 3);
  lv_obj_set_event_cb(playField, this->pressCallback);
  lv_obj_add_style(playField, LV_TABLE_PART_CELL1, &cellStyle);

  for (uint8_t column = 0; column < 3; column++) {
    lv_table_set_col_width(playField, column, LV_HOR_RES / 3);
  }

  for (uint8_t column = 0; column < 3; column++) {
    for (uint8_t row = 0; row < 3; row++) {
      lv_table_set_cell_align(playField, column, row, LV_LABEL_ALIGN_CENTER);
    }
  }

  lv_obj_align(playField, NULL, LV_ALIGN_CENTER, 0, 0);
  playField->user_data = this;
}

bool TicTacToe::checkCellValue(uint16_t column, uint16_t row, PlayerType player) {
  const char* cell_content = lv_table_get_cell_value(this->playField, column, row);
  if (strlen(cell_content) == 0 || this->charToPlayer(cell_content[0]) != player) {
    return false;
  }
  return true;
}

void TicTacToe::pressCallback(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_PRESSED) {
    TicTacToe* tic_tac_toe = static_cast<TicTacToe*>(obj->user_data);
    uint16_t column, row;
    lv_res_t result = lv_table_get_pressed_cell(tic_tac_toe->playField, &column, &row);

    if (result == LV_RES_INV) {
      return;
    }
    const char* cellContent = lv_table_get_cell_value(tic_tac_toe->playField, column, row);

    if (strlen(cellContent) != 0) {
      return;
    }

    if (tic_tac_toe->currentPlayer == PlayerType::X) {
      lv_table_set_cell_value(tic_tac_toe->playField, column, row, "X");
      tic_tac_toe->currentPlayer = PlayerType::O;
    } else {
      lv_table_set_cell_value(tic_tac_toe->playField, column, row, "O");
      tic_tac_toe->currentPlayer = PlayerType::X;
    }
    PlayerType hasWon = tic_tac_toe->hasWon();
    if (hasWon != PlayerType::UNKNOWN) {
      tic_tac_toe->showWin(hasWon);
    } else if (tic_tac_toe->hasEnded()) {
      tic_tac_toe->showDraw();
    }
  }
}

bool TicTacToe::checkPath(uint16_t startColumn, uint16_t startRow, uint16_t endColumn, uint16_t endRow, PlayerType player) {
  for (uint16_t column = startColumn; column <= endColumn; column++) {
    for (uint16_t row = startRow; row <= endRow; row++) {
      if (!this->checkCellValue(column, row, player)) {
        return false;
      }
    }
  }
  return true;
}

bool TicTacToe::checkPlayer(PlayerType player) {
  // Check Rows
  for (uint8_t row = 0; row <= 2; row++) {
    if (this->checkPath(0, row, 2, row, player)) {
      return true;
    }
  }

  // Check Columns
  for (uint8_t column = 0; column <= 2; column++) {
    if (this->checkPath(column, 0, column, 2, player)) {
      return true;
    }
  }

  // Check Diagonal
  return this->checkDiagonal(player);
}

bool TicTacToe::checkDiagonal(PlayerType player) {
  uint16_t row = 0;
  bool won = true;

  // Top-Left to Bottom-Right
  for (uint16_t column = 0; column <= 2; column++) {
    if (!this->checkCellValue(column, row, player)) {
      won = false;
      break;
    }
    row++;
  }

  if(won) {
    return true;
  }

  // Top-Right to Bottom-Left
  row = 2;
  for (uint16_t column = 0; column <= 2; column++) {
    if (!this->checkCellValue(column, row, player)) {
      return false;
    }
    row--;
  }
  return true;
}

TicTacToe::PlayerType TicTacToe::hasWon() {
  if (this->checkPlayer(PlayerType::X)) {
    return PlayerType::X;
  } else if (this->checkPlayer(PlayerType::O)) {
    return PlayerType::O;
  }
  return PlayerType::UNKNOWN;
}

bool TicTacToe::hasEnded() {
  for (uint16_t column = 0; column < 3; column++) {
    for (uint16_t row = 0; row < 3; row++) {
      const char* cellContent = lv_table_get_cell_value(this->playField, column, row);

      if (strlen(cellContent) == 0) {
        return false;
      }
    }
  }
  return true;
}

lv_obj_t* TicTacToe::createLabel(int16_t x_offset) {
  lv_obj_clean(lv_scr_act());

  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_width(label, LV_HOR_RES);
  lv_obj_set_height(label, LV_VER_RES);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, x_offset, 0);
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);

  return label;
}

void TicTacToe::showDraw() {
  lv_obj_t* label = this->createLabel(-30);
  lv_label_set_text_static(label, "Draw !!!");
}

void TicTacToe::showWin(PlayerType player) {
  lv_obj_t* label = this->createLabel(-32);
  lv_label_set_text_fmt(label, "%c Won !!!", player);
}

TicTacToe::~TicTacToe() {
  lv_style_reset(&cellStyle);
  lv_obj_clean(lv_scr_act());
}
