#include "displayapp/screens/Minesweeper.h"
#include "displayapp/DisplayApp.h"
#include <iostream>
#include <regex>
#include <string>
#include <cmath>
#include <sstream>
#include <array>
#include <algorithm>

using namespace Pinetime::Applications::Screens;

std::vector<bool> field(64, false);
static lv_style_t style_btn;
static lv_style_t style_clicked_btn;
static lv_style_t style_mine_btn;
static lv_style_t style_label;
uint16_t lastX = 0;
uint16_t lastY = 0;
bool isComplete = false;
bool ignoreFirst = false;

void lose() {
  if (!isComplete) {
    isComplete = true;
    lv_obj_t* obj = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_pos(obj, 20, 105);
    lv_obj_set_size(obj, 200, 30);
    lv_obj_add_style(obj, LV_OBJ_PART_MAIN, &style_label);

    lv_obj_t* label = lv_label_create(obj, nullptr);
    lv_label_set_text(label, "You lost...");
    lv_obj_align(label, nullptr, LV_ALIGN_CENTER, 0, 0);
  }
}

bool verifyCompletion() {
  lv_obj_t* child = lv_obj_get_child(lv_scr_act(), NULL);
  while (child != NULL) {
    if (lv_obj_get_child(child, nullptr) == nullptr) {
      return false;
    }
    child = lv_obj_get_child(lv_scr_act(), child);
  }
  if (!isComplete) {
    isComplete = true;
    lv_obj_t* obj = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_pos(obj, 20, 105);
    lv_obj_set_size(obj, 200, 30);
    lv_obj_add_style(obj, LV_OBJ_PART_MAIN, &style_label);

    lv_obj_t* label = lv_label_create(obj, nullptr);
    lv_label_set_text(label, "You won!");
    lv_obj_align(label, nullptr, LV_ALIGN_CENTER, 0, 0);
  }
  return true;
}

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Minesweeper*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

void makeField() {
  lv_style_init(&style_btn);
  lv_style_set_border_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_border_width(&style_btn, LV_STATE_DEFAULT, 1);
  lv_style_set_border_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_50);
  lv_style_set_radius(&style_btn, LV_STATE_DEFAULT, 0);
  lv_style_set_bg_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x52, 0x52, 0x52));

  int x = 0;
  int y = 0;
  for (int i = 1; i <= 64; i++) {
    lv_obj_t* btn = lv_btn_create(lv_scr_act(), nullptr);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, 30, 30);
    lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
    lv_obj_set_event_cb(btn, btnEventHandler);
    if (i % 8 == 0) {
      x = 0;
      y += 30;
    } else {
      x += 30;
    }
  }

  lv_style_init(&style_clicked_btn);
  lv_style_set_bg_color(&style_clicked_btn, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x82, 0x82, 0x82));

  lv_style_init(&style_mine_btn);
  lv_style_set_bg_color(&style_mine_btn, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x85, 0x35, 0x35));

  lv_style_init(&style_label);
  lv_style_set_border_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_border_width(&style_label, LV_STATE_DEFAULT, 3);
  lv_style_set_border_opa(&style_label, LV_STATE_DEFAULT, LV_OPA_70);
  lv_style_set_radius(&style_label, LV_STATE_DEFAULT, 10);
  lv_style_set_bg_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xa2, 0xa2, 0xa2));
}

void resetField() {
  lv_obj_t* child = lv_obj_get_child(lv_scr_act(), NULL);
  bool removeFirst = true;
  while (child != NULL) {
    if (removeFirst) {
      lv_obj_t* oldChild = child;
      child = lv_obj_get_child(lv_scr_act(), child);
      lv_obj_del(oldChild);
      removeFirst = false;
    } else {
      lv_obj_remove_style(child, LV_BTN_PART_MAIN, &style_clicked_btn);
      lv_obj_remove_style(child, LV_BTN_PART_MAIN, &style_mine_btn);
      lv_obj_clean(child);
      child = lv_obj_get_child(lv_scr_act(), child);
    }
  }
}

Minesweeper::Minesweeper() {
  makeField();
  unsigned char count = 0;
  while (count < 10) {
    unsigned char index = rand() % 64;
    if (!field[index]) {
      field[index] = true;
      count++;
    }
  }
}

Minesweeper::~Minesweeper() {
  lv_obj_clean(lv_scr_act());
}

int checkBombs(int x, int y) {
  int bombs = 0;
  for (int cx = x - 1; cx <= x + 1; cx++) {
    for (int cy = y - 1; cy <= y + 1; cy++) {
      if (cx >= 0 && cy >= 0 && cx < 8 && cy < 8) {
        if (field[cx + cy * 8] == true) {
          bombs++;
        }
      }
    }
  }
  return bombs;
}

void revealField(lv_obj_t* btn) {
  if (lv_obj_get_child(btn, nullptr) == nullptr) {
    int x = lv_obj_get_x(btn) / 30;
    int y = lv_obj_get_y(btn) / 30;
    lv_obj_t* label = lv_label_create(btn, nullptr);
    if (field[x + y * 8] == true) {
      lv_label_set_text(label, "\xEF\x87\xA2");
      lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_mine_btn);
      lose();
    } else {
      if (checkBombs(x, y) == 0) {
        lv_obj_t* child = lv_obj_get_child(lv_scr_act(), NULL);
        while (child != NULL) {
          int cx = lv_obj_get_x(child) / 30;
          int cy = lv_obj_get_y(child) / 30;
          if (cx >= x - 1 && cx <= x + 1 && cy >= y - 1 && cy <= y + 1) {
            if (checkBombs(cx, cy) != -1) {
              revealField(child);
            }
            if (cx == x - 1 && cy == y - 1) {
              break;
            }
          }
          child = lv_obj_get_child(lv_scr_act(), child);
        }
        lv_label_set_text(label, "");
      } else {
        lv_label_set_text(label, std::to_string(checkBombs(x, y)).c_str());
      }
      lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_clicked_btn);
      verifyCompletion();
    }
    lv_obj_align(label, nullptr, LV_ALIGN_CENTER, 0, 0);
  }
}

void flagField(lv_obj_t* btn) {
  if (lv_obj_get_child(btn, nullptr) == nullptr) {
    int x = lv_obj_get_x(btn) / 30;
    int y = lv_obj_get_y(btn) / 30;
    lv_obj_t* label = lv_label_create(btn, nullptr);
    lv_label_set_text(label, "\xEF\x80\xA4");
    lv_obj_align(label, nullptr, LV_ALIGN_CENTER, 0, 0);
  } else {
    lv_obj_t* child = lv_obj_get_child(btn, nullptr);
    lv_obj_del(child);
  }
}

void Minesweeper::OnButtonEvent(lv_obj_t* btn, lv_event_t event) {
  if (event == LV_EVENT_CLICKED && !isComplete)
    revealField(btn);
}

bool Minesweeper::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (event == Pinetime::Applications::TouchEvents::LongTap) {
    lv_obj_t* child = lv_obj_get_child(lv_scr_act(), NULL);
    while (child != NULL) {
      int x = lv_obj_get_x(child) / 30;
      int y = lv_obj_get_y(child) / 30;
      if (x == lastX && y == lastY) {
        flagField(child);
        verifyCompletion();
        break;
      }
      child = lv_obj_get_child(lv_scr_act(), child);
    }
    return true;
  }
  return false;
}

bool Minesweeper::OnTouchEvent(uint16_t x, uint16_t y) {
  lastX = x / 30;
  lastY = y / 30;
  if (isComplete) {
    isComplete = false;
    resetField();
    field.assign(64, false);
    unsigned char count = 0;
    while (count < 10) {
      unsigned char index = rand() % 64;
      if (!field[index]) {
        field[index] = true;
        count++;
      }
    }
  }
  return true;
}