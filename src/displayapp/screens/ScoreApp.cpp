#include "ScoreApp.h"
#include "Screen.h"
#include "Symbols.h"
#include "lvgl/lvgl.h"

using namespace Pinetime::Applications::Screens;


static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto *screen = static_cast<ScoreApp*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

ScoreApp::widget_t ScoreApp::createButton(lv_align_t alignment, uint8_t x, uint8_t y, uint8_t width, uint8_t height, const char text[]){

  widget_t wdg = {nullptr, nullptr};

  wdg.button = lv_btn_create(lv_scr_act(), nullptr);
  wdg.button->user_data = this;
  lv_obj_set_event_cb(wdg.button, btnEventHandler);
  lv_obj_align(wdg.button, lv_scr_act(), alignment, x, y);
  lv_obj_set_size(wdg.button, width, height);
  wdg.label = lv_label_create(wdg.button, nullptr);
  lv_label_set_text(wdg.label, text);

  return wdg;
}

ScoreApp::ScoreApp(DisplayApp* app, Controllers::MotorController& motorController) : Screen(app),  motorController {motorController}{

  score1Wdg = createButton(LV_ALIGN_IN_TOP_LEFT, 0, 0, scoreWidth, scoreHeight, "0");
  lv_obj_set_style_local_text_font(score1Wdg.label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(score1Wdg.label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  score2Wdg = createButton(LV_ALIGN_IN_TOP_LEFT, 0, +scoreHeight, scoreWidth, scoreHeight, "0");
  lv_obj_set_style_local_text_font(score2Wdg.label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(score2Wdg.label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  
  sets1Wdg = createButton(LV_ALIGN_IN_TOP_LEFT, 5, 5, 20, 20, "0");
  sets2Wdg = createButton(LV_ALIGN_IN_TOP_LEFT, 5, 5+scoreHeight, 20, 20, "0"); // TODO(toitoinou) align with score2

  lv_label_set_text_fmt(score1Wdg.label, "%d", score[0].points);
  lv_label_set_text_fmt(score2Wdg.label, "%d", score[1].points);
  
  score1SecondaryWdg = createButton(LV_ALIGN_IN_TOP_RIGHT, 0, 0, minusWidth, minusHeight, "-1");
  score2SecondaryWdg = createButton(LV_ALIGN_IN_TOP_RIGHT, 0, displayHeight-minusHeight, minusWidth, minusHeight, "-1");

  modeDropdownWdg = lv_dropdown_create(lv_scr_act(), nullptr);
  modeDropdownWdg->user_data = this;
  lv_obj_set_event_cb(modeDropdownWdg, btnEventHandler);
  // lv_obj_set_style_local_pad_left(modeDropdownWdg, LV_DROPDOWN_PART_MAIN, LV_STATE_DEFAULT, 20);
  // lv_obj_set_style_local_pad_left(modeDropdownWdg, LV_DROPDOWN_PART_LIST, LV_STATE_DEFAULT, 20);
  lv_obj_set_size(modeDropdownWdg, modeWidth, modeHeight);
  lv_obj_align(modeDropdownWdg, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, minusHeight);
  lv_dropdown_set_options(modeDropdownWdg, "+1\nBad\nTen\n0");
  lv_dropdown_set_selected(modeDropdownWdg, 0);
  lv_dropdown_set_show_selected(modeDropdownWdg, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  // lv_dropdown_set_text(modeDropdownWdg, "mode");

  // TODO(toitoinou): add active mode icon in empty area
}

ScoreApp::~ScoreApp() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void ScoreApp::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    if (obj == modeDropdownWdg) {
      if (lv_dropdown_get_selected(obj) == ScoreApp::RESET){
        score[0] = {0, 0, 0};
        score[1] = {0, 0, 0};
      } else {
        mode = static_cast<ScoreApp::mode_t>(lv_dropdown_get_selected(obj));
      }
    }
  }
  else if (event == LV_EVENT_CLICKED) {
    if (obj == score1Wdg.button) {
      scoreMainButtonAction(0);
    }
    else if (obj == score2Wdg.button) {
      scoreMainButtonAction(1);
    }
    else if (obj == score1SecondaryWdg.button) {
     scoreSecondaryButtonAction(0);
    }
    else if (obj == score2SecondaryWdg.button) {
      scoreSecondaryButtonAction(1);
    }
  }

  // Refresh screen
  lv_label_set_text_fmt(score1Wdg.label, "%d", score[0].points);
  lv_label_set_text_fmt(score2Wdg.label, "%d", score[1].points);
  lv_label_set_text_fmt(sets1Wdg.label, "%d", score[0].sets);
  lv_label_set_text_fmt(sets2Wdg.label, "%d", score[1].sets);
}

  void ScoreApp::scoreMainButtonAction(uint8_t scoreId) {
    switch (mode) {
      case(ScoreApp::BADMINTON):
        if (score[scoreId].points < 21) {
          score[scoreId].points++;
        } else { // end of set
          // save current score
          // TODO(toitoinou) will not work to retrieve 2 sets, arrays could be used to store evolution of the game
          for (uint8_t i : {0, 1}){
            score[i].oldPoints = score[i].points;
            score[i].oldSets = score[i].sets;
            score[i].points = 0;
          }
          score[scoreId].sets++;
          motorController.RunForDuration(30);
        }
        break;
      case(ScoreApp::SIMPLE_COUNTER):
      default:
        score[scoreId].points < 255 ? score[scoreId].points++ : 255; // manage overflow
        break;
  }
}

  void ScoreApp::scoreSecondaryButtonAction(uint8_t scoreId){
  switch (mode) {
    case ScoreApp::BADMINTON:
      if (score[scoreId].points > 0){
        score[scoreId].points--;
      } else {
        // retrieve all old points
        if ( (score[0].points == 0) && (score[1].points == 0) ) {
          for (uint8_t i : {0, 1}){
            score[i].points = score[i].oldPoints;
            score[i].sets = score[i].oldSets;
          }
        } // else nothing is done
      }
    break;
    case(ScoreApp::SIMPLE_COUNTER):
    default:
      score[scoreId].points > 0 ? score[scoreId].points-- : 0; // manage underflow
    break;
  }
}
