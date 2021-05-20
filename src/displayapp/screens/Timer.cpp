#include "Timer.h"

#include "Screen.h"
#include "Symbols.h"
#include "lvgl/lvgl.h"


using namespace Pinetime::Applications::Screens;


static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  Timer* screen = static_cast<Timer*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

void Timer::createButtons() {
  btnMinutesUp = lv_btn_create(lv_scr_act(), nullptr);
  btnMinutesUp->user_data = this;
  lv_obj_set_event_cb(btnMinutesUp, btnEventHandler);
  lv_obj_align(btnMinutesUp, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 20, -80);
  lv_obj_set_height(btnMinutesUp, 40);
  lv_obj_set_width(btnMinutesUp, 60);
  txtMUp = lv_label_create(btnMinutesUp, nullptr);
  lv_label_set_text(txtMUp, "+");
  
  btnMinutesDown = lv_btn_create(lv_scr_act(), nullptr);
  btnMinutesDown->user_data = this;
  lv_obj_set_event_cb(btnMinutesDown, btnEventHandler);
  lv_obj_align(btnMinutesDown, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 20, +40);
  lv_obj_set_height(btnMinutesDown, 40);
  lv_obj_set_width(btnMinutesDown, 60);
  txtMDown = lv_label_create(btnMinutesDown, nullptr);
  lv_label_set_text(txtMDown, "-");
  
  btnSecondsUp = lv_btn_create(lv_scr_act(), nullptr);
  btnSecondsUp->user_data = this;
  lv_obj_set_event_cb(btnSecondsUp, btnEventHandler);
  lv_obj_align(btnSecondsUp, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 10, -80);
  lv_obj_set_height(btnSecondsUp, 40);
  lv_obj_set_width(btnSecondsUp, 60);
  txtSUp = lv_label_create(btnSecondsUp, nullptr);
  lv_label_set_text(txtSUp, "+");
  
  btnSecondsDown = lv_btn_create(lv_scr_act(), nullptr);
  btnSecondsDown->user_data = this;
  lv_obj_set_event_cb(btnSecondsDown, btnEventHandler);
  lv_obj_align(btnSecondsDown, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 10, +40);
  lv_obj_set_height(btnSecondsDown, 40);
  lv_obj_set_width(btnSecondsDown, 60);
  txtSDown = lv_label_create(btnSecondsDown, nullptr);
  lv_label_set_text(txtSDown, "-");
  
}


Timer::Timer(DisplayApp* app, Controllers::TimerController& timerController)
    : Screen(app),
      running{true},
      timerController{timerController} {
  
  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  
  uint32_t seconds = timerController.GetTimeRemaining() / 1000;
  lv_label_set_text_fmt(time, "%02d:%02d", seconds / 60, seconds % 60);
  
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -20);
  
  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, btnEventHandler);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_height(btnPlayPause, 40);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  if (timerController.IsRunning()) {
    lv_label_set_text(txtPlayPause, Symbols::pause);
  } else {
    lv_label_set_text(txtPlayPause, Symbols::play);
    createButtons();
  }
  
}

Timer::~Timer() {
  lv_obj_clean(lv_scr_act());
  
}

bool Timer::Refresh() {
  if (timerController.IsRunning()) {
    uint32_t seconds = timerController.GetTimeRemaining() / 1000;
    lv_label_set_text_fmt(time, "%02d:%02d", seconds / 60, seconds % 60);
  }
  return running;
}

void Timer::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnPlayPause) {
      if (timerController.IsRunning()) {
        lv_label_set_text(txtPlayPause, Symbols::play);
        uint32_t seconds = timerController.GetTimeRemaining() / 1000;
        minutesToSet = seconds / 60;
        secondsToSet = seconds % 60;
        timerController.StopTimer();
        createButtons();
        
      } else if (secondsToSet + minutesToSet > 0) {
        lv_label_set_text(txtPlayPause, Symbols::pause);
        timerController.StartTimer((secondsToSet + minutesToSet * 60) * 1000);
        
        lv_obj_del(btnSecondsDown);
        btnSecondsDown = nullptr;
        lv_obj_del(btnSecondsUp);
        btnSecondsUp = nullptr;
        lv_obj_del(btnMinutesDown);
        btnMinutesDown = nullptr;
        lv_obj_del(btnMinutesUp);
        btnMinutesUp = nullptr;
        
      }
    } else {
      if (!timerController.IsRunning()) {
        if (obj == btnMinutesUp) {
          if (minutesToSet >= 59) {
            minutesToSet = 0;
          } else {
            minutesToSet++;
          }
          lv_label_set_text_fmt(time, "%02d:%02d", minutesToSet, secondsToSet);
          
        } else if (obj == btnMinutesDown) {
          if (minutesToSet == 0) {
            minutesToSet = 59;
          } else {
            minutesToSet--;
          }
          lv_label_set_text_fmt(time, "%02d:%02d", minutesToSet, secondsToSet);
          
        } else if (obj == btnSecondsUp) {
          if (secondsToSet >= 59) {
            secondsToSet = 0;
          } else {
            secondsToSet++;
          }
          lv_label_set_text_fmt(time, "%02d:%02d", minutesToSet, secondsToSet);
          
        } else if (obj == btnSecondsDown) {
          if (secondsToSet == 0) {
            secondsToSet = 59;
          } else {
            secondsToSet--;
          }
          lv_label_set_text_fmt(time, "%02d:%02d", minutesToSet, secondsToSet);
          
        }
      }
      
    }
    
  }
  
}


void Timer::setDone() {
  lv_label_set_text(time, "00:00");
  lv_label_set_text(txtPlayPause, Symbols::play);
  secondsToSet = 0;
  minutesToSet = 0;
  createButtons();
}