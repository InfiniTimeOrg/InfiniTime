#include "displayapp/screens/Magic8ball.h"

// How big the 3d acceleration vector needs to be to be taken as a shake
#define SHAKE_THRESHOLD 2048
// If two taps are gotten within this many ticks, consider it a double tap and refresh the response
#define DOUBLE_TAP_TIME_TICKS pdMS_TO_TICKS(500)

// After a shake is taken, how long the screen should stay black before starting to fade in (can be set to 0)
#define FADEIN_PAUSE_TICKS pdMS_TO_TICKS(200)
// Once pause is finished, fade in (can be set to 0)
#define FADEIN_FADE_TICKS pdMS_TO_TICKS(300)
// Smooth fading (or as good as the watch can do) looks bad. Fade in with this many discrete steps.
// (ex: pause=100ms, fade=150ms, steps=4 -> 0ms: 0%, 100ms: 25%, 150ms: 50%, 200ms: 75%, 250ms: 100%, no interpolation.)
#define FADEIN_STEP_COUNT 3

using namespace Pinetime::Applications::Screens;

namespace {
  void EventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Magic8ball*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

Magic8ball::Magic8ball(Controllers::MotionController& motionController)
  : motionController{motionController} {

  activePool = 0; // default pool
  isMenuOpen = false;
  fadeStartTimestamp = 0;
  lastClickTimestamp = xTaskGetTickCount() - DOUBLE_TAP_TIME_TICKS;
  triangleDisplayState = Displaying;
  lastTriangleFadeColor = LV_COLOR_BLUE;
  std::srand(xTaskGetTickCount());

  // Outline for the big blue background triangle
  lv_style_init(&bigBlueTriangleOutlineStyle);
  lv_style_set_line_width(&bigBlueTriangleOutlineStyle, LV_STATE_DEFAULT, 16);
  lv_style_set_line_color(&bigBlueTriangleOutlineStyle, LV_STATE_DEFAULT, LV_COLOR_BLUE);
  lv_style_set_line_rounded(&bigBlueTriangleOutlineStyle, LV_STATE_DEFAULT, true);
  bigBlueTriangleOutline = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(bigBlueTriangleOutline, triangleOutlinePoints, 4);
  lv_obj_add_style(bigBlueTriangleOutline, LV_LINE_PART_MAIN, &bigBlueTriangleOutlineStyle);

  // Fill for the big blue background triangle
  // Consists of a mask with two lines matching the bottom left and bottom right slopes, and a child basic object
  //  which matches the size of the background triangle.
  lv_obj_t* bigBlueTriangleFillMask = lv_objmask_create(lv_scr_act(), nullptr);
  lv_obj_set_size(bigBlueTriangleFillMask, LV_HOR_RES, LV_VER_RES);
  lv_obj_align(bigBlueTriangleFillMask, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_draw_mask_line_param_t bigBlueTriangleFillMaskLineL, bigBlueTriangleFillMaskLineR;
  lv_draw_mask_line_points_init(&bigBlueTriangleFillMaskLineL,
                                triangleOutlinePoints[0].x,
                                triangleOutlinePoints[0].y,
                                triangleOutlinePoints[2].x,
                                triangleOutlinePoints[2].y,
                                LV_DRAW_MASK_LINE_SIDE_RIGHT);
  lv_draw_mask_line_points_init(&bigBlueTriangleFillMaskLineR,
                                triangleOutlinePoints[1].x,
                                triangleOutlinePoints[1].y,
                                triangleOutlinePoints[2].x,
                                triangleOutlinePoints[2].y,
                                LV_DRAW_MASK_LINE_SIDE_LEFT);
  lv_objmask_add_mask(bigBlueTriangleFillMask, &bigBlueTriangleFillMaskLineL);
  lv_objmask_add_mask(bigBlueTriangleFillMask, &bigBlueTriangleFillMaskLineR);
  lv_style_init(&bigBlueTriangleFillStyle);
  lv_style_set_radius(&bigBlueTriangleFillStyle, LV_STATE_DEFAULT, 0);
  bigBlueTriangleFill = lv_obj_create(bigBlueTriangleFillMask, nullptr);
  lv_obj_set_size(bigBlueTriangleFill,
                  triangleOutlinePoints[1].x - triangleOutlinePoints[0].x,
                  triangleOutlinePoints[2].y - triangleOutlinePoints[0].y);
  lv_obj_set_style_local_bg_color(bigBlueTriangleFill, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);
  lv_obj_add_style(bigBlueTriangleFill, LV_LINE_PART_MAIN, &bigBlueTriangleFillStyle);
  lv_obj_set_pos(bigBlueTriangleFill, triangleOutlinePoints[0].x, triangleOutlinePoints[0].y);

  // Main text (centered 5/8 up from the bottom of the triangle)
  mainText = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(mainText, helpText);
  lv_label_set_align(mainText, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(mainText, bigBlueTriangleFill, LV_ALIGN_CENTER, 0, -(triangleOutlinePoints[2].y - triangleOutlinePoints[0].y) / 8);

  // Button to close long tap menu
  btnCloseMenu = lv_btn_create(lv_scr_act(), nullptr);
  btnCloseMenu->user_data = this;
  lv_obj_set_size(btnCloseMenu, 60, 60);
  lv_obj_align(btnCloseMenu, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 15, 15);
  lv_obj_set_style_local_bg_opa(btnCloseMenu, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_t* lblClose = lv_label_create(btnCloseMenu, nullptr);
  lv_label_set_text_static(lblClose, "X");
  lv_obj_set_event_cb(btnCloseMenu, EventHandler);
  lv_obj_set_hidden(btnCloseMenu, true);

  // Button to go to next pool in long tap menu
  btnPoolNext = lv_btn_create(lv_scr_act(), nullptr);
  btnPoolNext->user_data = this;
  lv_obj_set_size(btnPoolNext, 60, 60);
  lv_obj_align(btnPoolNext, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 0);
  lv_obj_set_style_local_bg_opa(btnPoolNext, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_t* lblNext = lv_label_create(btnPoolNext, nullptr);
  lv_label_set_text_static(lblNext, ">");
  lv_obj_set_event_cb(btnPoolNext, EventHandler);
  lv_obj_set_hidden(btnPoolNext, true);

  // Button to go to previous pool in long tap menu
  btnPoolPrev = lv_btn_create(lv_scr_act(), nullptr);
  btnPoolPrev->user_data = this;
  lv_obj_set_size(btnPoolPrev, 60, 60);
  lv_obj_align(btnPoolPrev, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 0);
  lv_obj_set_style_local_bg_opa(btnPoolPrev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_t* lblPrev = lv_label_create(btnPoolPrev, nullptr);
  lv_label_set_text_static(lblPrev, "<");
  lv_obj_set_event_cb(btnPoolPrev, EventHandler);
  lv_obj_set_hidden(btnPoolPrev, true);

  // Background for current pool text in long tap menu
  currentPoolBG = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(currentPoolBG, 210, 60);
  lv_obj_align(currentPoolBG, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -15);
  lv_obj_set_style_local_bg_opa(currentPoolBG, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  // Copy the color from whatever the default color for a button is
  lv_obj_set_style_local_bg_color(currentPoolBG,
                                  LV_OBJ_PART_MAIN,
                                  LV_STATE_DEFAULT,
                                  lv_obj_get_style_bg_color(btnCloseMenu, LV_BTN_PART_MAIN));

  // Current pool text in long tap menu
  currentPoolText = lv_label_create(currentPoolBG, nullptr);
  lv_label_set_text_static(currentPoolText, answerPools[activePool].GetName());
  lv_label_set_align(currentPoolText, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(currentPoolText, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_hidden(currentPoolBG, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

Magic8ball::~Magic8ball() {
  lv_style_reset(&bigBlueTriangleOutlineStyle);
  lv_style_reset(&bigBlueTriangleFillStyle);
  lv_obj_clean(lv_scr_act());
  lv_task_del(taskRefresh);
}

bool Magic8ball::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case TouchEvents::Tap:
      if (isMenuOpen) {
        return false;
      }
      if (xTaskGetTickCount() - lastClickTimestamp < DOUBLE_TAP_TIME_TICKS) {
        Start8BallRefresh();
      }
      lastClickTimestamp = xTaskGetTickCount();
      return true;
    case TouchEvents::LongTap:
      if (!isMenuOpen) {
        OpenMenu();
        return true;
      }
      return false;
    default:
      return false;
  }
}

bool Magic8ball::OnButtonPushed() {
  if (isMenuOpen) {
    CloseMenu();
    return true;
  }
  return false;
}

void Magic8ball::Refresh() {
  if (triangleDisplayState == Displaying) {
    // Disable shaking if not fading in and menu is open
    if (isMenuOpen) {
      return;
    }
    // Find 3d pythagorean theorem for actual total acceleration
    lv_sqrt_res_t sqrtResult;
    _lv_sqrt(_lv_pow(motionController.X(), 2) + _lv_pow(motionController.Y(), 2), &sqrtResult, 0x8000);
    _lv_sqrt(_lv_pow(motionController.Z(), 2) + _lv_pow(sqrtResult.i, 2), &sqrtResult, 0x8000);
    if (sqrtResult.i > SHAKE_THRESHOLD) {
      Start8BallRefresh();
      // For screen timeout purposes, act as though shaking the watch was a user click
      lv_disp_trig_activity(nullptr);
    }
  } else if (triangleDisplayState == FadingIn) {
    const uint32_t fadeProgression = xTaskGetTickCount() - fadeStartTimestamp;

    lv_color_t newColor;
    if (fadeProgression < FADEIN_PAUSE_TICKS) {
      // In pause period
      newColor = LV_COLOR_BLACK;
    } else if (fadeProgression < FADEIN_PAUSE_TICKS + FADEIN_FADE_TICKS) {
      // In stepping period, do interpolation between black and blue
      // stepProgress is integer in range [1,FADEIN_STEP_COUNT)
      uint32_t stepProgress = 1 + ((fadeProgression - FADEIN_PAUSE_TICKS) * (FADEIN_STEP_COUNT - 1) / FADEIN_FADE_TICKS);
      newColor = LV_COLOR_MAKE(0, 0, 255 * stepProgress / FADEIN_STEP_COUNT);
    } else {
      // Fade completed
      newColor = LV_COLOR_BLUE;
      triangleDisplayState = Displaying;
      lv_obj_set_hidden(mainText, false);
      lv_label_set_text_static(mainText, getRandomString());
      lv_obj_align(mainText, bigBlueTriangleFill, LV_ALIGN_CENTER, 0, -(triangleOutlinePoints[2].y - triangleOutlinePoints[0].y) / 8);
    }

    // Commit the new color to the triangle if it's been changed
    if (newColor.full != lastTriangleFadeColor.full) {
      lastTriangleFadeColor = newColor;
      lv_obj_set_style_local_bg_color(bigBlueTriangleFill, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, newColor);
      lv_style_set_line_color(&bigBlueTriangleOutlineStyle, LV_OBJ_PART_MAIN, newColor);
      lv_obj_refresh_style(bigBlueTriangleOutline, LV_OBJ_PART_MAIN, LV_STYLE_LINE_COLOR);
    }
  }
}

const char* Magic8ball::getRandomString() const {
  // Get sum of weights in current pool
  uint16_t poolTotalWeight = 0;
  const Pool* selectedPool{&answerPools[activePool]};
  for (size_t i = 0; i < selectedPool->size(); i++) {
    poolTotalWeight += (*selectedPool)[i].weight;
  }

  // Have total weights in the pool, choose something within that now
  uint16_t selectedIndex = std::rand() % poolTotalWeight;
  CategoryType selectedCategory = None;
  for (size_t i = 0; i < selectedPool->size(); i++) {
    if (selectedIndex < (*selectedPool)[i].weight) {
      selectedCategory = (*selectedPool)[i].categoryType;
      break;
    }
    selectedIndex -= (*selectedPool)[i].weight;
  }

  // Now need to randomly pick within selectedCategory
  return categories.at(selectedCategory)[std::rand() % categories.at(selectedCategory).size()];
}

void Magic8ball::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (object == btnCloseMenu) {
      CloseMenu();
    }
    if (object == btnPoolNext) {
      activePool++;
      if (activePool >= static_cast<int>(answerPools.size())) {
        activePool = 0;
      }
      lv_label_set_text_static(currentPoolText, answerPools[activePool].GetName());
      lv_obj_align(currentPoolText, nullptr, LV_ALIGN_CENTER, 0, 0);
    }
    if (object == btnPoolPrev) {
      activePool--;
      if (activePool < 0) {
        activePool = answerPools.size() - 1;
      }
      lv_label_set_text_static(currentPoolText, answerPools[activePool].GetName());
      lv_obj_align(currentPoolText, nullptr, LV_ALIGN_CENTER, 0, 0);
    }
  }
}

void Magic8ball::OpenMenu() {
  if (isMenuOpen) {
    return;
  }
  isMenuOpen = true;
  lv_obj_set_hidden(btnCloseMenu, false);
  lv_obj_set_hidden(btnPoolNext, false);
  lv_obj_set_hidden(btnPoolPrev, false);
  lv_obj_set_hidden(currentPoolBG, false);
}

void Magic8ball::CloseMenu() {
  if (!isMenuOpen) {
    return;
  }
  isMenuOpen = false;
  lv_obj_set_hidden(btnCloseMenu, true);
  lv_obj_set_hidden(btnPoolNext, true);
  lv_obj_set_hidden(btnPoolPrev, true);
  lv_obj_set_hidden(currentPoolBG, true);
}

void Magic8ball::Start8BallRefresh() {
  // Extra safety, calling code should take care of this though
  if (isMenuOpen || triangleDisplayState == FadingIn) {
    return;
  }
  // Set state to fading in
  triangleDisplayState = FadingIn;
  fadeStartTimestamp = xTaskGetTickCount();
  lv_obj_set_hidden(mainText, true);
}