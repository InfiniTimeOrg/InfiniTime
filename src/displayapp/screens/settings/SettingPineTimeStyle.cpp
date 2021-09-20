#include "SettingPineTimeStyle.h"
#include <lvgl/lvgl.h>
#include <displayapp/Colors.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t* obj, lv_event_t event) {
    SettingPineTimeStyle* screen = static_cast<SettingPineTimeStyle*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingPineTimeStyle::SettingPineTimeStyle(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : PineTimeStyleBase(app), settingsController {settingsController} {

  CreateObjects(Convert(settingsController.GetPTSColorBG()), Convert(settingsController.GetPTSColorTime()), Convert(settingsController.GetPTSColorBar()));

  btnNextTime = lv_btn_create(lv_scr_act(), nullptr);
  btnNextTime->user_data = this;
  lv_obj_set_size(btnNextTime, 60, 60);
  lv_obj_align(btnNextTime, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, -80);
  lv_obj_set_style_local_bg_opa(btnNextTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
  lv_obj_set_style_local_value_str(btnNextTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, ">");
  lv_obj_set_event_cb(btnNextTime, event_handler);

  btnPrevTime = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevTime->user_data = this;
  lv_obj_set_size(btnPrevTime, 60, 60);
  lv_obj_align(btnPrevTime, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, -80);
  lv_obj_set_style_local_bg_opa(btnPrevTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
  lv_obj_set_style_local_value_str(btnPrevTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "<");
  lv_obj_set_event_cb(btnPrevTime, event_handler);

  btnNextBar = lv_btn_create(lv_scr_act(), nullptr);
  btnNextBar->user_data = this;
  lv_obj_set_size(btnNextBar, 60, 60);
  lv_obj_align(btnNextBar, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 0);
  lv_obj_set_style_local_bg_opa(btnNextBar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
  lv_obj_set_style_local_value_str(btnNextBar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, ">");
  lv_obj_set_event_cb(btnNextBar, event_handler);

  btnPrevBar = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevBar->user_data = this;
  lv_obj_set_size(btnPrevBar, 60, 60);
  lv_obj_align(btnPrevBar, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 0);
  lv_obj_set_style_local_bg_opa(btnPrevBar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
  lv_obj_set_style_local_value_str(btnPrevBar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "<");
  lv_obj_set_event_cb(btnPrevBar, event_handler);

  btnNextBG = lv_btn_create(lv_scr_act(), nullptr);
  btnNextBG->user_data = this;
  lv_obj_set_size(btnNextBG, 60, 60);
  lv_obj_align(btnNextBG, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 80);
  lv_obj_set_style_local_bg_opa(btnNextBG, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
  lv_obj_set_style_local_value_str(btnNextBG, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, ">");
  lv_obj_set_event_cb(btnNextBG, event_handler);

  btnPrevBG = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevBG->user_data = this;
  lv_obj_set_size(btnPrevBG, 60, 60);
  lv_obj_align(btnPrevBG, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 80);
  lv_obj_set_style_local_bg_opa(btnPrevBG, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
  lv_obj_set_style_local_value_str(btnPrevBG, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "<");
  lv_obj_set_event_cb(btnPrevBG, event_handler);

  btnReset = lv_btn_create(lv_scr_act(), nullptr);
  btnReset->user_data = this;
  lv_obj_set_size(btnReset, 60, 60);
  lv_obj_align(btnReset, lv_scr_act(), LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_local_bg_opa(btnReset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
  lv_obj_set_style_local_value_str(btnReset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Rst");
  lv_obj_set_event_cb(btnReset, event_handler);

  btnRandom = lv_btn_create(lv_scr_act(), nullptr);
  btnRandom->user_data = this;
  lv_obj_set_size(btnRandom, 60, 60);
  lv_obj_align(btnRandom, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(btnRandom, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
  lv_obj_set_style_local_value_str(btnRandom, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Rnd");
  lv_obj_set_event_cb(btnRandom, event_handler);
}

SettingPineTimeStyle::~SettingPineTimeStyle() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingPineTimeStyle::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  auto valueTime = settingsController.GetPTSColorTime();
  auto valueBar = settingsController.GetPTSColorBar();
  auto valueBG = settingsController.GetPTSColorBG();

  if (event == LV_EVENT_CLICKED) {
    if (object == btnNextTime) {
      valueTime = GetNext(valueTime);

      settingsController.SetPTSColorTime(valueTime);
      lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
    }
    if (object == btnPrevTime) {
      valueTime = GetPrevious(valueTime);
      settingsController.SetPTSColorTime(valueTime);
      lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
    }
    if (object == btnNextBar) {
      valueBar = GetNext(valueBar);
      if(valueBar == Controllers::Settings::Colors::Black)
        valueBar = GetNext(valueBar);
      settingsController.SetPTSColorBar(valueBar);
      lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBar));
    }
    if (object == btnPrevBar) {
      valueBar = GetPrevious(valueBar);
      if(valueBar == Controllers::Settings::Colors::Black)
        valueBar = GetPrevious(valueBar);
      settingsController.SetPTSColorBar(valueBar);
      lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBar));
    }
    if (object == btnNextBG) {
      valueBG = GetNext(valueBG);
      settingsController.SetPTSColorBG(valueBG);
      lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBG));
    }
    if (object == btnPrevBG) {
      valueBG = GetPrevious(valueBG);
      settingsController.SetPTSColorBG(valueBG);
      lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBG));
    }
    if (object == btnReset) {
      settingsController.SetPTSColorTime(Controllers::Settings::Colors::Teal);
      lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Teal));
      lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Teal));
      lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Teal));
      settingsController.SetPTSColorBar(Controllers::Settings::Colors::Teal);
      lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Teal));
      settingsController.SetPTSColorBG(Controllers::Settings::Colors::Black);
      lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Black));
    }
    if (object == btnRandom) {
      uint8_t randTime = rand() % 17;
      uint8_t randBar = rand() % 17;
      uint8_t randBG = rand() % 17;
      // Check if the time color is the same as its background, or if the sidebar is black. If so, change them to more useful values.
      if (randTime == randBG) {
        randBG += 1;
      }
      if (randBar == 3) {
        randBar -= 1;
      }
      settingsController.SetPTSColorTime(static_cast<Controllers::Settings::Colors>(randTime));
      lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(static_cast<Controllers::Settings::Colors>(randTime)));
      lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,  Convert(static_cast<Controllers::Settings::Colors>(randTime)));
      lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,  Convert(static_cast<Controllers::Settings::Colors>(randTime)));
      settingsController.SetPTSColorBar(static_cast<Controllers::Settings::Colors>(randBar));
      lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT,  Convert(static_cast<Controllers::Settings::Colors>(randBar)));
      settingsController.SetPTSColorBG(static_cast<Controllers::Settings::Colors>(randBG));
      lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT,  Convert(static_cast<Controllers::Settings::Colors>(randBG)));
    }
  }
}

Pinetime::Controllers::Settings::Colors SettingPineTimeStyle::GetNext(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors nextColor;
  if (colorAsInt < 16) {
    nextColor = static_cast<Controllers::Settings::Colors>(colorAsInt + 1);
  } else {
    nextColor = static_cast<Controllers::Settings::Colors>(0);
  }
  return nextColor;
}

Pinetime::Controllers::Settings::Colors SettingPineTimeStyle::GetPrevious(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors prevColor;

  if (colorAsInt > 0) {
    prevColor = static_cast<Controllers::Settings::Colors>(colorAsInt - 1);
  } else {
    prevColor = static_cast<Controllers::Settings::Colors>(16);
  }
  return prevColor;
}
