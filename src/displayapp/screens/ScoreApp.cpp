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

ScoreApp::ScoreApp(DisplayApp* app) : Screen(app){
    
  score1Wdg = createButton(LV_ALIGN_IN_TOP_LEFT, 0, 0, scoreWidth, scoreHeight, "0");
  lv_obj_set_style_local_text_font(score1Wdg.label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(score1Wdg.label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  score2Wdg = createButton(LV_ALIGN_IN_TOP_LEFT, 0, +scoreHeight, scoreWidth, scoreHeight, "0");
  lv_obj_set_style_local_text_font(score2Wdg.label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(score2Wdg.label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  lv_label_set_text_fmt(score1Wdg.label, "%d", score1.points);
  lv_label_set_text_fmt(score2Wdg.label, "%d", score2.points);
  
  score1SecondaryWdg = createButton(LV_ALIGN_IN_TOP_RIGHT, 0, 0, minusWidth, minusHeight, "-1");
  score2SecondaryWdg = createButton(LV_ALIGN_IN_TOP_RIGHT, 0, displayHeight-minusHeight, minusWidth, minusHeight, "-1");
  
  resetWdg = createButton(LV_ALIGN_IN_TOP_RIGHT, 0, displayHeight/2, resetWidth, resetHeight, "Rst");

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

ScoreApp::~ScoreApp() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void ScoreApp::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == score1Wdg.button) {
      score1 = scoreMainButtonAction(score1);
    }
    else if (obj == score2Wdg.button) {
      score2 = scoreMainButtonAction(score2);
    }
    else if (obj == score1SecondaryWdg.button) {
     score1 = scoreSecondaryButtonAction(score1);
    }
    else if (obj == score2SecondaryWdg.button) {
      score2 = scoreSecondaryButtonAction(score2);
    }
    else if (obj == resetWdg.button) {
      score1 = {0, 0, 0};
      score2 = {0, 0, 0};
    }
    lv_label_set_text_fmt(score1Wdg.label, "%d", score1.points);
    lv_label_set_text_fmt(score2Wdg.label, "%d", score2.points);
  }
}

ScoreApp::score_t ScoreApp::scoreMainButtonAction(ScoreApp::score_t score){
  switch (mode) {
    case(ScoreApp::BADMINTON):
      score.points < 21 ? score.points++ : 0;
    case(ScoreApp::SIMPLE_COUNTER):
    default:
      score.points < 255 ? score.points++ : 255; // manage overflow
      break;
  }
   

   return score;  
}

ScoreApp::score_t ScoreApp::scoreSecondaryButtonAction(ScoreApp::score_t score){
  switch (mode) {
    case ScoreApp::BADMINTON:
      score.points > 0 ? score.points-- : 0; // manage underflow
    case(ScoreApp::SIMPLE_COUNTER):
    default:
      score.points > 0 ? score.points-- : 0; // manage underflow
      break;
  }
  return score;
}
