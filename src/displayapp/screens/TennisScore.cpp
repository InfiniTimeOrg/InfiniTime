#include "TennisScore.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

static void topUpBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  auto* tennis = static_cast<TennisScore*>(obj->user_data);
  tennis->IncTop();
}

static void topDownBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  auto* tennis = static_cast<TennisScore*>(obj->user_data);
  tennis->DecTop();
}

static void bottomUpBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  auto* tennis = static_cast<TennisScore*>(obj->user_data);
  tennis->IncBottom();
}

static void bottomDownBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  auto* tennis = static_cast<TennisScore*>(obj->user_data);
  tennis->DecBottom();
}

TennisScore::TennisScore(DisplayApp* app): Screen(app)
{
  Widgets::ScoreBoard board = Widgets::ScoreBoard();
  int offset = board.Width * -1;

  lv_obj_t* container = gameBoard.Create(game);
  lv_obj_align(container, nullptr, LV_ALIGN_IN_LEFT_MID, padding,0);

  int rPad = padding *-1;
  container = set3Board.Create(set3);
  lv_obj_align(container, nullptr, LV_ALIGN_IN_RIGHT_MID, rPad ,0);
  container = set2Board.Create(set2);
  lv_obj_align(container, nullptr, LV_ALIGN_IN_RIGHT_MID, offset+rPad,0);
  offset += offset;
  container = set1Board.Create(set1);
  lv_obj_align(container, nullptr, LV_ALIGN_IN_RIGHT_MID, offset+rPad,0);

  int plusSize = LV_HOR_RES * 0.5 - (padding *2);
  int minusSize = LV_HOR_RES * 0.4 - (padding *2);

  auto BtnCreate = [&](lv_obj_t *parent, char * text, int width, int height, lv_color_t color)
  {
    lv_obj_t* btn = lv_btn_create(parent, nullptr);
    btn->user_data = this;
    lv_obj_set_height(btn, height);
    lv_obj_set_width(btn, width);
    lv_obj_t* btnLabel = lv_label_create(btn, nullptr);
    lv_obj_align(btnLabel, nullptr, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text_static(btnLabel, text);
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, color);

    return btn;
  };

  lv_color_t green = LV_COLOR_MAKE(0x0, 0xb0, 0x0);
  int negPad = padding *-1;
  lv_obj_t* topUpBtn = BtnCreate(lv_scr_act(), plus, plusSize, btnHeight, green);
  lv_obj_set_event_cb(topUpBtn, topUpBtnEventHandler);
  lv_obj_align(topUpBtn, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, padding, padding);

  lv_obj_t* topDownBtn = BtnCreate(lv_scr_act(), minus, minusSize, btnHeight, LV_COLOR_RED);
  lv_obj_set_event_cb(topDownBtn, topDownBtnEventHandler);
  lv_obj_align(topDownBtn, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, negPad, padding);

  lv_obj_t* bottomUpBtn = BtnCreate(lv_scr_act(), plus, plusSize, btnHeight, green);
  lv_obj_set_event_cb(bottomUpBtn, bottomUpBtnEventHandler);
  lv_obj_align(bottomUpBtn, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, padding, negPad);

  lv_obj_t* bottomDownBtn = BtnCreate(lv_scr_act(), minus, minusSize, btnHeight, LV_COLOR_RED);
  lv_obj_set_event_cb(bottomDownBtn, bottomDownBtnEventHandler);
  lv_obj_align(bottomDownBtn, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, negPad, negPad);

  updateSetColors();
}

TennisScore::~TennisScore() {
  lv_obj_clean(lv_scr_act());
}

void TennisScore::updateSetColors() {
  set1Board.SetColor(LV_COLOR_GRAY);
  set2Board.SetColor(LV_COLOR_GRAY);
  set3Board.SetColor(LV_COLOR_GRAY);

  switch (currentSet) {
    case 1:
       set1Board.SetColor(LV_COLOR_SILVER);
       break;
    case 2:
       set2Board.SetColor(LV_COLOR_SILVER);
       break;
    case 3:
       set3Board.SetColor(LV_COLOR_SILVER);
       break;
    default:
      break;
  }
}

void TennisScore::IncTop() {
  uint8_t top = getTopGameScore();
  uint8_t bottom = getBottomGameScore();
  if (isWin && top > bottom) {
    addToSet();
    return;
  }
  if (top < 99) {
    top++;
    setTopGameScore(top);
    updateGameText();
  }
}

void TennisScore::DecTop() {
  uint8_t top = getTopGameScore();
  if (top > 0) {
    top--;
    setTopGameScore(top);
    updateGameText();
  }
}

void TennisScore::IncBottom() {
  uint8_t top = getTopGameScore();
  uint8_t bottom = getBottomGameScore();
  if (isWin && bottom > top) {
    addToSet();
    return;
  }
  if (bottom < 99) {
    bottom++;
    setBottomGameScore(bottom);
    updateGameText();
  }
}

void TennisScore::DecBottom() {
  uint8_t bottom = getBottomGameScore();
  if (bottom > 0) {
    bottom--;
    setBottomGameScore(bottom);
    updateGameText();
  }
}

void TennisScore::addToSet() {

  uint8_t topSet = getTopSetScore();
  uint8_t bottomSet = getBottomSetScore();

  if (getTopGameScore() > getBottomGameScore()) {
      topSet++;
  } else {
     bottomSet++;
  }

  setTopSetScore(topSet);
  setBottomSetScore(bottomSet);
  setTopGameScore(0);
  setBottomGameScore(0);

  if (tieBreak && isWin) {
    tieBreak = false;
    if (currentSet < 3) {
      currentSet++;
    }
  } else {
    uint8_t setDiff = std::abs(topSet - bottomSet);
    if (topSet > 5 || bottomSet > 5) {
      if (setDiff < 2) {
        if ((topSet > 6 || bottomSet > 6)
          || (topSet == 6 && bottomSet == 6)) {
          tieBreak = true;
        }
      } else {
        if (currentSet < 3) {
          currentSet++;
        }
      }
    }
  }

  updateGameText();
  updateSetText();
  updateSetColors();

  isWin = false;
}

void TennisScore::updateSetText() {
  set1Board.SetTopText(scores[2]);
  set1Board.SetBottomText(scores[3]);
  set2Board.SetTopText(scores[4]);
  set2Board.SetBottomText(scores[5]);
  set3Board.SetTopText(scores[6]);
  set3Board.SetBottomText(scores[7]);
}


uint8_t TennisScore::getTopGameScore() {
  return scores[0];
}

uint8_t TennisScore::getBottomGameScore() {
  return scores[1];
}

void TennisScore::setTopGameScore(uint8_t score) {
  scores[0] = score;
}

void TennisScore::setBottomGameScore(uint8_t score) {
  scores[1] = score;
}


uint8_t TennisScore::getTopSetScore() {
  return scores[currentSet*2];
}

uint8_t TennisScore::getBottomSetScore() {
  return scores[currentSet*2+1];
}

void TennisScore::setTopSetScore(uint8_t score) {
  scores[currentSet*2] = score;
}

void TennisScore::setBottomSetScore(uint8_t score) {
  scores[currentSet*2+1] = score;
}

void TennisScore::updateGameText() {

  uint8_t top = getTopGameScore();
  uint8_t bottom = getBottomGameScore();

  uint8_t diff = std::abs(top-bottom);

  isWin = false;
  if (tieBreak)
  {
    gameBoard.SetTopText(top);
    gameBoard.SetBottomText(bottom);
    if (top > 6 || bottom > 6) {
      flagWinner(top, bottom);
    }
  } else {
    if (top <= 3 && bottom <= 3) {
      top = std::min(top * 15, 40);
      bottom = std::min(bottom * 15, 40);

      gameBoard.SetTopText(top);
      gameBoard.SetBottomText(bottom);
    }
    else {
      gameBoard.SetTopText(minus);
      gameBoard.SetBottomText(minus);

      if (diff < 2) {
        // show advantage
        if (top > bottom) {
          gameBoard.SetTopText(advantage);
        } else if (bottom > top)  {
          gameBoard.SetBottomText(advantage);
        }
      } else {
        flagWinner(top,bottom);
      }
    }
  }
}

void TennisScore::flagWinner(uint8_t top, uint8_t bottom) {
  // show winner
  isWin = true;
  if (top > bottom) {
    gameBoard.SetTopText(win);
  } else {
    gameBoard.SetBottomText(win);
  }
}
