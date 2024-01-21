#include "displayapp/screens/CatDog.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/LittleVgl.h"
#include "displayapp/screens/Symbols.h"

#include <cstdlib> // for rand()

using namespace Pinetime::Applications::Screens;

CatDog::CatDog() {

  lv_obj_t* background = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(background, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_style_local_radius(background, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_bg_color(background, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x154360));

  info = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(info, "Touch to Start");
  lv_obj_align(info, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 40);

  bone = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(bone, Symbols::bone);
  lv_obj_set_pos(bone, 240, 240);

  cat = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(cat, Symbols::cat);
  lv_obj_set_style_local_text_color(cat, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xa2d9ce));
  lv_obj_set_pos(cat, 215, 195);

  dog = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(dog, Symbols::dog);
  lv_obj_set_style_local_text_color(dog, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xf7dc6f));
  lv_obj_set_pos(dog, 5, 195);

  windTxt = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(windTxt, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
  lv_label_set_text_static(windTxt, "");

  windBarBG = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(windBarBG, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_size(windBarBG, 120, 4);
  lv_obj_set_pos(windBarBG, 240, 240);

  windBar = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(windBar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);
  lv_obj_set_pos(windBar, 240, 240);

  powerBar = lv_bar_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_bg_color(powerBar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_style_local_bg_opa(powerBar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
  lv_obj_set_style_local_bg_color(powerBar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_bar_set_range(powerBar, MIN_POWER, MAX_POWER);
  lv_obj_set_size(powerBar, 120, 4);
  lv_obj_set_pos(powerBar, 240, 240);

  catLifeBar = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(catLifeBar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

  dogLifeBar = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(dogLifeBar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

  UpdateLife();

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

CatDog::~CatDog() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void CatDog::Refresh() {
  switch (state) {
    case CatDog::State::Idle:
      // add a delay to reject previous press event
      if (animationTime > 0) {
        animationTime--;
      } else {
        if (isGameStopped) {
          state = CatDog::State::Stopped;
        } else {
          state = CatDog::State::Prompt;
        }
      }
      break;
    case CatDog::State::Stopped:
      // start game
      if (isPressing) {
        lv_obj_align(windBarBG, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 30);
        catLifePoint = 100;
        dogLifePoint = 100;
        UpdateLife();
        isDogsTurn = true;
        animationTime = 20;
        isGameStopped = false;
        state = CatDog::State::Idle;
      }
      break;
    case CatDog::State::Prompt:
      RandomWind();
      if (isDogsTurn) {
        lv_label_set_text_static(info, "Your turn");
        lv_obj_align(info, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 40);
        state = CatDog::State::WaitUser;
      } else {
        lv_label_set_text_static(info, "Cat's turn");
        lv_obj_align(info, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 40);
        lv_obj_align(powerBar, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 70);
        // throwPower for perfect hit is 370 + wind*12;
        PCTargetThrowPower = 343 + wind * 6 + rand() % 54;
        lv_obj_set_pos(bone, 215, HORIZON / 100);
        state = CatDog::State::ComputerThrowing;
      }
      break;
    case CatDog::State::WaitUser:
      if (isPressing) {
        lv_obj_align(powerBar, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 70); // show powerBar
        lv_obj_set_pos(bone, 5, HORIZON / 100);
        state = CatDog::State::UserThrowing;
      }
      break;
    case CatDog::State::UserThrowing:
      if ((!isPressing && throwPower > MIN_POWER + 20) || throwPower >= MAX_POWER) {
        Throw(throwPower);
      } else {
        throwPower += 3;
        lv_bar_set_value(powerBar, throwPower, LV_ANIM_OFF);
      }
      break;
    case CatDog::State::ComputerThrowing:
      if (throwPower < PCTargetThrowPower) {
        throwPower += 3;
        lv_bar_set_value(powerBar, throwPower, LV_ANIM_OFF);
      } else {
        Throw(-throwPower);
      }
      break;
    case CatDog::State::BoneFlying:
      boneVY += 33;   // gravity
      boneVX += wind; // acceleration caused by wind
      boneX += boneVX;
      boneY += boneVY;
      lv_obj_set_pos(bone, boneX / 100, boneY / 100);
      if (boneY >= HORIZON || boneX > 24000 || boneX < -2400) {
        if (-2400 < boneX && boneX < 24000) {
          if (isDogsTurn) {
            handleDamage(abs(boneX - 21500) / 100);
          } else {
            handleDamage(abs(boneX - 500) / 100);
          }
        } else {
          handleDamage(100); // missed
        }
      }
      break;
    case CatDog::State::CatHit:
      if (animationTime > 0) {
        animationTime--;
        if (animationTime % 4 == 0) {
          lv_obj_set_style_local_text_color(cat, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xa2d9ce));
        } else if (animationTime % 4 == 2) {
          lv_obj_set_style_local_text_color(cat, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
        }
      } else {
        state = CatDog::State::Result;
      }
      break;
    case CatDog::State::DogHit:
      if (animationTime > 0) {
        animationTime--;
        if (animationTime % 4 == 0) {
          lv_obj_set_style_local_text_color(dog, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xf7dc6f));
        } else if (animationTime % 4 == 2) {
          lv_obj_set_style_local_text_color(dog, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
        }
      } else {
        state = CatDog::State::Result;
      }
      break;
    case CatDog::State::Missed:
      if (animationTime > 0) {
        animationTime--;
      } else {
        state = CatDog::State::Result;
      }
      break;
    case CatDog::State::Result:
      if (dogLifePoint == 0) {
        lv_label_set_text_static(info, "Cat Won!");
        lv_obj_align(info, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 40);
        animationTime = 20;
        isGameStopped = true;
        state = CatDog::State::Idle;
      } else if (catLifePoint == 0) {
        lv_label_set_text_static(info, "You Won!");
        lv_obj_align(info, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 40);
        animationTime = 20;
        isGameStopped = true;
        state = CatDog::State::Idle;
      } else {
        isDogsTurn = !isDogsTurn;
        state = CatDog::State::Prompt;
      }
      break;
  }
  isPressing = false;
}

void CatDog::Throw(int16_t speed) {
  lv_obj_set_pos(powerBar, 240, 240); // hide it
  boneY = HORIZON;
  boneVX = speed;
  boneVY = 0;
  if (speed > 0) {
    boneX = 500;
    boneVY -= 25 * speed / 10;
  } else {
    // 100*(240-BONE_SIZE-5)
    boneX = 21500;
    boneVY += 25 * speed / 10;
  }
  throwPower = MIN_POWER;
  state = CatDog::State::BoneFlying;
}

void CatDog::handleDamage(uint8_t distance) {
  if (distance < 30) {
    // animationTime is propotional to damage
    animationTime = (50 - distance);
    if (isDogsTurn) {
      catLifePoint -= animationTime / 2;
      state = CatDog::State::CatHit;
    } else {
      dogLifePoint -= animationTime / 2;
      state = CatDog::State::DogHit;
    }
  } else {
    animationTime = 50;
    lv_label_set_text_static(info, "Missed");
    lv_obj_align(info, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 40);
    state = CatDog::State::Missed;
  }
  UpdateLife();
}

void CatDog::RandomWind() {
  wind = rand() % 9 - 4; // -4 to 4
  lv_obj_set_size(windBar, abs(wind) * 15, 4);
  lv_obj_set_pos(windBar, 50, 30);
  lv_obj_align(windBar, lv_scr_act(), LV_ALIGN_IN_TOP_MID, wind * 15 / 2, 30);

  if (wind > 0) {
    lv_label_set_text_static(windTxt, " wind>");
  } else if (wind < 0) {
    lv_label_set_text_static(windTxt, "<wind ");
  } else {
    lv_label_set_text_static(windTxt, " wind ");
  }
  lv_obj_align(windTxt, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
}

void CatDog::UpdateLife() {
  if (catLifePoint > 100) {
    catLifePoint = 0;
  }
  if (dogLifePoint > 100) {
    dogLifePoint = 0;
  }
  lv_obj_set_size(catLifeBar, catLifePoint, 2);
  lv_obj_align(catLifeBar, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, -2);
  lv_obj_set_size(dogLifeBar, dogLifePoint, 2);
  lv_obj_align(dogLifeBar, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, -2);
}

bool CatDog::OnTouchEvent(Pinetime::Applications::TouchEvents /*event*/) {
  isPressing = true;
  return true;
}

bool CatDog::OnTouchEvent(uint16_t /*x*/, uint16_t /*y*/) {
  isPressing = true;
  return true;
}
