#include "ScoreApp.h"
#include "Screen.h"
#include "Symbols.h"
#include "lvgl/lvgl.h"

using namespace Pinetime::Applications::Screens;


static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  ScoreApp* screen = static_cast<ScoreApp*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

ScoreApp::widget_t ScoreApp::_createButton(lv_align_t alignment, uint8_t x, uint8_t y, uint8_t width, uint8_t height, const char text[]){

  widget_t button;

  button.button = lv_btn_create(lv_scr_act(), nullptr);
  button.button->user_data = this;
  lv_obj_set_event_cb(button.button, btnEventHandler);
  lv_obj_align(button.button, lv_scr_act(), alignment, x, y);
  lv_obj_set_width(button.button, width);
  lv_obj_set_height(button.button, height);
  button.label = lv_label_create(button.button, nullptr);
  lv_label_set_text(button.label, text);

  return button;
}

ScoreApp::ScoreApp(DisplayApp* app) : Screen(app) {
    
  btnMyScore = _createButton(LV_ALIGN_IN_TOP_LEFT, 0, 0, scoreWidth, scoreHeight, "0");
  btnYourScore = _createButton(LV_ALIGN_IN_BOTTOM_LEFT, 0, 0, scoreWidth, scoreHeight, "0");
  lv_label_set_text_fmt(btnMyScore.label, "%d", myScore);
  lv_label_set_text_fmt(btnYourScore.label, "%d", yourScore);
  
  btnMyScoreMinus = _createButton(LV_ALIGN_IN_TOP_RIGHT, 0, 0, minusWidth, minusHeight, "-1");
  btnYourScoreMinus = _createButton(LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0, minusWidth, minusHeight, "-1");
  
  btnReset = _createButton(LV_ALIGN_CENTER, 20, 0, resetWidth, resetHeight, "Rst");

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

ScoreApp::~ScoreApp() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void ScoreApp::Refresh() {
  lv_label_set_text_fmt(btnMyScore.label, "%d", myScore);
  lv_label_set_text_fmt(btnYourScore.label, "%d", yourScore);
}

void ScoreApp::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnMyScore.button)
        myScore++;
    else if (obj == btnYourScore.button) {
      yourScore++;
    }
    else if (obj == btnMyScoreMinus.button) {
      myScore--;
    }
    else if (obj == btnYourScoreMinus.button) {
      yourScore--;
    }
    else if (obj == btnReset.button) {
      myScore = 0;
        yourScore = 0;
    }
  }
}
