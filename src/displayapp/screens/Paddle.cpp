#include "Paddle.h"
#include "../DisplayApp.h"
#include "../LittleVgl.h"

using namespace Pinetime::Applications::Screens;

Paddle::Paddle(Pinetime::Applications::DisplayApp* app, Pinetime::Components::LittleVgl& lvgl) : Screen(app), lvgl {lvgl} {
  background = lv_obj_create(lv_scr_act());
  lv_obj_set_size(background, LV_HOR_RES + 1, LV_VER_RES);
  lv_obj_set_pos(background, -1, 0);
  lv_obj_set_style_radius(background, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(background, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(background, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(background, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

  points = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(points, &jetbrains_mono_42, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(points, "0000");
  lv_obj_align(points, LV_ALIGN_TOP_MID, 0, 10);

  paddle = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(paddle, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(paddle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(paddle, 4, 60);

  ball = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(ball, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ball, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ball, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(ball, ballSize, ballSize);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, this);
}

Paddle::~Paddle() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Paddle::Refresh() {
  ballX += dx;
  ballY += dy;

  lv_obj_set_pos(ball, ballX, ballY);

  // checks if it has touched the sides (floor and ceiling)
  if (ballY <= 1 || ballY >= LV_VER_RES - ballSize - 2) {
    dy *= -1;
  }

  // checks if it has touched the side (left side)
  if (ballX >= LV_VER_RES - ballSize - 1) {
    dx *= -1;
  }

  // checks if it is in the position of the paddle
  if (dx < 0 && ballX <= 4) {
    if (ballX >= -ballSize / 4) {
      if (ballY <= (paddlePos + 30 - ballSize / 4) && ballY >= (paddlePos - 30 - ballSize + ballSize / 4)) {
        dx *= -1;
        score++;
      }
    }
    // checks if it has gone behind the paddle
    else if (ballX <= -ballSize * 2) {
      ballX = (LV_HOR_RES - ballSize) / 2;
      ballY = (LV_VER_RES - ballSize) / 2;
      score = 0;
    }
  }
  lv_label_set_text_fmt(points, "%04d", score);
}

bool Paddle::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return true;
}

bool Paddle::OnTouchEvent(uint16_t x, uint16_t y) {
  // sets the center paddle pos. (30px offset) with the the y_coordinate of the finger
  lv_obj_set_pos(paddle, 0, y - 30);
  paddlePos = y;
  return true;
}
