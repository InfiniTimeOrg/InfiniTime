#include "displayapp/screens/Bird.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/LittleVgl.h"
#include "displayapp/screens/Symbols.h"

#include <cstdlib> // for rand()

using namespace Pinetime::Applications::Screens;

Bird::Bird() {

  lv_obj_t* background = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(background, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_style_local_radius(background, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_bg_color(background, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x2874a6));

  cactus_top = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(cactus_top, CACTUS_WIDTH, CACTUS_HEIGHT);
  lv_obj_set_style_local_bg_color(cactus_top, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_obj_set_style_local_border_color(cactus_top, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_border_width(cactus_top, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);

  cactus_bottom = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(cactus_bottom, CACTUS_WIDTH, CACTUS_HEIGHT);
  lv_obj_set_style_local_bg_color(cactus_bottom, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_obj_set_style_local_border_color(cactus_bottom, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_border_width(cactus_bottom, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);

  MovePipe();

  points = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(points, "    ");
  lv_obj_align(points, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -10, 5);

  info = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(info, "touch to start");
  lv_obj_align(info, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 40);

  bird = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(bird, Symbols::dove);
  lv_obj_set_style_local_text_color(bird, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_obj_set_pos(bird, BIRD_X, pos);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Bird::~Bird() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Bird::MovePipe() {
  lv_obj_set_pos(cactus_top, cactus_x, -cactus_y_offset);
  lv_obj_set_pos(cactus_bottom, cactus_x, CACTUS_HEIGHT + CACTUS_GAP - cactus_y_offset);
  return;
}

void Bird::Refresh() {
  if (is_stopped) {
    return;
  }

  pos += accel / 6;

  if (is_ascending) {
    if (accel >= -18) {
      accel -= 3;
    }
  } else {
    if (accel <= 42) {
      accel++;
    }
  }

  // checks if it has hit the floor or ceiling
  if (pos <= 1 || pos >= LV_VER_RES - BIRD_SIZE) {
    GameOver();
    return;
  }

  // checks if it has rammed into cacti
  // BIRD_X-CACTUS_WIDTH to BIRD_X+BIRD_SIZE
  if (90 < cactus_x && cactus_x < 130) {
    if (pos < CACTUS_HEIGHT - cactus_y_offset || pos > CACTUS_HEIGHT + CACTUS_GAP - BIRD_SIZE - cactus_y_offset) {
      GameOver();
      return;
    }
  }

  lv_obj_set_pos(bird, BIRD_X, pos);

  lv_label_set_text_fmt(points, "%04d", score / 10);
  is_ascending = false;

  score++;
  if (cactus_x == 0) {
    while (true) {
      uint8_t new_offset = rand() % 5 * 40;
      if (new_offset != cactus_y_offset) {
        cactus_y_offset = new_offset;
        break;
      }
    }
    cactus_x = 240;
  }
  cactus_x--;
  if (cactus_x % 4 == 0) {
    MovePipe();
  }
}

void Bird::GameOver() {
  is_stopped = true;
  lv_label_set_text_static(info, "Game Over");
  lv_obj_align(info, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 40);
  lv_obj_set_style_local_text_color(bird, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  return;
}

bool Bird::OnTouchEvent(Pinetime::Applications::TouchEvents /*event*/) {
  if (is_stopped) {
    if (pos != 120) {
      pos = 120;
      lv_label_set_text_static(info, "Touch to Start");
      lv_obj_align(info, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 40);
      lv_obj_set_style_local_text_color(bird, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
      return true;
    }
    // reset
    cactus_x = 240;
    accel = 0;
    score = 0;
    is_stopped = false;
    lv_label_set_text_static(info, "");
  }
  is_ascending = true;
  return true;
}

bool Bird::OnTouchEvent(uint16_t /*x*/, uint16_t /*y*/) {
  is_ascending = true;
  return true;
}
