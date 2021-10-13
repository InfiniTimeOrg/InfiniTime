#include "SettingPineTimeStyle.h"
#include <lvgl/lvgl.h>
#include <displayapp/Colors.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_event_t *event) {
    SettingPineTimeStyle* screen = static_cast<SettingPineTimeStyle*>(lv_event_get_user_data(event));
    screen->UpdateSelected(lv_event_get_target(event), event);
  }
}

SettingPineTimeStyle::SettingPineTimeStyle(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController} {
  timebar = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(timebar, Convert(settingsController.GetPTSColorBG()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(timebar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(timebar, 200, 240);
  lv_obj_align(timebar, LV_ALIGN_TOP_LEFT, 5, 0);

  // Display the time
  timeDD1 = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(timeDD1, &open_sans_light, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(timeDD1, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(timeDD1, "12");
  lv_obj_align_to(timeDD1, timebar, LV_ALIGN_TOP_MID, 5, 5);

  timeDD2 = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(timeDD2, &open_sans_light, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(timeDD2, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(timeDD2, "34");
  lv_obj_align_to(timeDD2, timebar, LV_ALIGN_BOTTOM_MID, 5, -5);

  timeAMPM = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(timeAMPM, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_line_space(timeAMPM, -3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(timeAMPM, "A\nM");
  lv_obj_align_to(timeAMPM, timebar, LV_ALIGN_BOTTOM_LEFT, 2, -20);

  // Create a 40px wide bar down the right side of the screen
  sidebar = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(sidebar, Convert(settingsController.GetPTSColorBar()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(sidebar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(sidebar, 40, 240);
  lv_obj_align(sidebar, LV_ALIGN_TOP_RIGHT, 0, 0);

  // Display icons
  batteryIcon = lv_label_create(sidebar);
  lv_obj_set_style_text_color(batteryIcon, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, LV_ALIGN_TOP_MID, 0, 2);

  bleIcon = lv_label_create(sidebar);
  lv_obj_set_style_text_color(bleIcon, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(bleIcon, LV_ALIGN_TOP_MID, 0, 25);

  // Calendar icon
  calendarOuter = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarOuter, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarOuter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarOuter, 34, 34);
  lv_obj_align_to(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 0);

  calendarInner = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarInner, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarInner, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarInner, 27, 27);
  lv_obj_align_to(calendarInner, calendarOuter, LV_ALIGN_CENTER, 0, 0);

  calendarBar1 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarBar1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarBar1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarBar1, 3, 12);
  lv_obj_align_to(calendarBar1, calendarOuter, LV_ALIGN_TOP_MID, -6, -3);

  calendarBar2 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarBar2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarBar2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarBar2, 3, 12);
  lv_obj_align_to(calendarBar2, calendarOuter, LV_ALIGN_TOP_MID, 6, -3);

  calendarCrossBar1 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarCrossBar1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarCrossBar1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarCrossBar1, 8, 3);
  lv_obj_align_to(calendarCrossBar1, calendarBar1, LV_ALIGN_BOTTOM_MID, 0, 0);

  calendarCrossBar2 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarCrossBar2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarCrossBar2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarCrossBar2, 8, 3);
  lv_obj_align_to(calendarCrossBar2, calendarBar2, LV_ALIGN_BOTTOM_MID, 0, 0);

  // Display date
  dateDayOfWeek = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(dateDayOfWeek, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(dateDayOfWeek, "THU");
  lv_obj_align_to(dateDayOfWeek, sidebar, LV_ALIGN_CENTER, 0, -34);

  dateDay = lv_label_create(calendarInner);
  lv_obj_set_style_text_color(dateDay, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(dateDay, "25");
  lv_obj_center(dateDay);

  dateMonth = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(dateMonth, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(dateMonth, "MAR");
  lv_obj_align_to(dateMonth, sidebar, LV_ALIGN_CENTER, 0, 32);

  // Step count gauge
  stepCont = lv_obj_create(lv_scr_act());
  lv_obj_set_size(stepCont, 40, 40);
  lv_obj_align(stepCont, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  
  needle_color = lv_color_white();
  stepMeter = lv_meter_create(stepCont);
  lv_obj_set_size(stepMeter, 37, 37);
  lv_obj_center(stepMeter);

  stepScale = lv_meter_add_scale(stepMeter);
  stepIndicator = lv_meter_add_needle_line(stepMeter, stepScale, 2, needle_color, -6);
  lv_meter_set_scale_range(stepMeter, stepScale, 0, 100, 360, 180);
  lv_meter_set_scale_ticks(stepMeter, stepScale, 11, 4, 4, lv_color_black());
  lv_meter_set_indicator_value(stepMeter, stepIndicator, 0);

  lv_obj_set_style_pad_all(stepMeter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(stepMeter, LV_OPA_TRANSP, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(stepMeter, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_line_color(stepMeter, lv_color_black(), LV_PART_TICKS | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(stepCont, LV_OPA_TRANSP, 0);

  backgroundLabel = lv_label_create(lv_scr_act());
  lv_obj_add_flag(backgroundLabel, LV_OBJ_FLAG_CLICKABLE);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");

  btnNextTime = lv_btn_create(lv_scr_act());
  btnNextTime->user_data = this;
  lv_obj_set_size(btnNextTime, 60, 60);
  lv_obj_align(btnNextTime, LV_ALIGN_RIGHT_MID, -15, -80);
  lv_obj_set_style_bg_opa(btnNextTime, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
  txtNextTime = lv_label_create(btnNextTime);
  lv_label_set_text(txtNextTime, ">");
  lv_obj_center(txtNextTime);
  lv_obj_add_event_cb(btnNextTime, event_handler, LV_EVENT_ALL, btnNextTime->user_data);

  btnPrevTime = lv_btn_create(lv_scr_act());
  btnPrevTime->user_data = this;
  lv_obj_set_size(btnPrevTime, 60, 60);
  lv_obj_align(btnPrevTime, LV_ALIGN_LEFT_MID, 15, -80);
  lv_obj_set_style_bg_opa(btnPrevTime, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
  txtPrevTime = lv_label_create(btnPrevTime);
  lv_label_set_text(txtPrevTime, "<");
  lv_obj_center(txtPrevTime);
  lv_obj_add_event_cb(btnPrevTime, event_handler, LV_EVENT_ALL, btnPrevTime->user_data);

  btnNextBar = lv_btn_create(lv_scr_act());
  btnNextBar->user_data = this;
  lv_obj_set_size(btnNextBar, 60, 60);
  lv_obj_align(btnNextBar, LV_ALIGN_RIGHT_MID, -15, 0);
  lv_obj_set_style_bg_opa(btnNextBar, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
  txtNextBar = lv_label_create(btnNextBar);
  lv_label_set_text(txtNextBar, ">");
  lv_obj_center(txtNextBar);
  lv_obj_add_event_cb(btnNextBar, event_handler, LV_EVENT_ALL, btnNextBar->user_data);

  btnPrevBar = lv_btn_create(lv_scr_act());
  btnPrevBar->user_data = this;
  lv_obj_set_size(btnPrevBar, 60, 60);
  lv_obj_align(btnPrevBar, LV_ALIGN_LEFT_MID, 15, 0);
  lv_obj_set_style_bg_opa(btnPrevBar, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
  txtPrevBar = lv_label_create(btnPrevBar);
  lv_label_set_text(txtPrevBar, "<");
  lv_obj_center(txtPrevBar);
  lv_obj_add_event_cb(btnPrevBar, event_handler, LV_EVENT_ALL, btnPrevBar->user_data);

  btnNextBG = lv_btn_create(lv_scr_act());
  btnNextBG->user_data = this;
  lv_obj_set_size(btnNextBG, 60, 60);
  lv_obj_align(btnNextBG, LV_ALIGN_RIGHT_MID, -15, 80);
  lv_obj_set_style_bg_opa(btnNextBG, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
  txtNextBG = lv_label_create(btnNextBG);
  lv_label_set_text(txtNextBG, ">");
  lv_obj_center(txtNextBG);
  lv_obj_add_event_cb(btnNextBG, event_handler, LV_EVENT_ALL, btnNextBG->user_data);

  btnPrevBG = lv_btn_create(lv_scr_act());
  btnPrevBG->user_data = this;
  lv_obj_set_size(btnPrevBG, 60, 60);
  lv_obj_align(btnPrevBG, LV_ALIGN_LEFT_MID, 15, 80);
  lv_obj_set_style_bg_opa(btnPrevBG, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
  txtPrevBG = lv_label_create(btnPrevBG);
  lv_label_set_text(txtPrevBG, "<");
  lv_obj_center(txtPrevBG);
  lv_obj_add_event_cb(btnPrevBG, event_handler, LV_EVENT_ALL, btnPrevBG->user_data);

  btnReset = lv_btn_create(lv_scr_act());
  btnReset->user_data = this;
  lv_obj_set_size(btnReset, 60, 60);
  lv_obj_align(btnReset, LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_bg_opa(btnReset, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
  txtReset = lv_label_create(btnReset);
  lv_label_set_text(txtReset, "Rst");
  lv_obj_center(txtReset);
  lv_obj_add_event_cb(btnReset, event_handler, LV_EVENT_ALL, btnReset->user_data);

  btnRandom = lv_btn_create(lv_scr_act());
  btnRandom->user_data = this;
  lv_obj_set_size(btnRandom, 60, 60);
  lv_obj_align(btnRandom, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_opa(btnRandom, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
  txtRandom = lv_label_create(btnRandom);
  lv_label_set_text(txtRandom, "Rnd");
  lv_obj_center(txtRandom);
  lv_obj_add_event_cb(btnRandom, event_handler, LV_EVENT_ALL, btnRandom->user_data);
}

SettingPineTimeStyle::~SettingPineTimeStyle() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingPineTimeStyle::UpdateSelected(lv_obj_t* object, lv_event_t* event) {
  auto valueTime = settingsController.GetPTSColorTime();
  auto valueBar = settingsController.GetPTSColorBar();
  auto valueBG = settingsController.GetPTSColorBG();

  if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
    if (object == btnNextTime) {
      valueTime = GetNext(valueTime);

      settingsController.SetPTSColorTime(valueTime);
      lv_obj_set_style_text_color(timeDD1, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeDD2, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeAMPM, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnPrevTime) {
      valueTime = GetPrevious(valueTime);
      settingsController.SetPTSColorTime(valueTime);
      lv_obj_set_style_text_color(timeDD1, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeDD2, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeAMPM, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnNextBar) {
      valueBar = GetNext(valueBar);
      if(valueBar == Controllers::Settings::Colors::Black)
        valueBar = GetNext(valueBar);
      settingsController.SetPTSColorBar(valueBar);
      lv_obj_set_style_bg_color(sidebar, Convert(valueBar), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnPrevBar) {
      valueBar = GetPrevious(valueBar);
      if(valueBar == Controllers::Settings::Colors::Black)
        valueBar = GetPrevious(valueBar);
      settingsController.SetPTSColorBar(valueBar);
      lv_obj_set_style_bg_color(sidebar, Convert(valueBar), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnNextBG) {
      valueBG = GetNext(valueBG);
      settingsController.SetPTSColorBG(valueBG);
      lv_obj_set_style_bg_color(timebar, Convert(valueBG), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnPrevBG) {
      valueBG = GetPrevious(valueBG);
      settingsController.SetPTSColorBG(valueBG);
      lv_obj_set_style_bg_color(timebar, Convert(valueBG), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnReset) {
      settingsController.SetPTSColorTime(Controllers::Settings::Colors::Teal);
      lv_obj_set_style_text_color(timeDD1, Convert(Controllers::Settings::Colors::Teal), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeDD2, Convert(Controllers::Settings::Colors::Teal), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeAMPM, Convert(Controllers::Settings::Colors::Teal), LV_PART_MAIN | LV_STATE_DEFAULT);
      settingsController.SetPTSColorBar(Controllers::Settings::Colors::Teal);
      lv_obj_set_style_bg_color(sidebar, Convert(Controllers::Settings::Colors::Teal), LV_PART_MAIN | LV_STATE_DEFAULT);
      settingsController.SetPTSColorBG(Controllers::Settings::Colors::Black);
      lv_obj_set_style_bg_color(timebar, Convert(Controllers::Settings::Colors::Black), LV_PART_MAIN | LV_STATE_DEFAULT);
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
      lv_obj_set_style_text_color(timeDD1, Convert(static_cast<Controllers::Settings::Colors>(randTime)), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeDD2,  Convert(static_cast<Controllers::Settings::Colors>(randTime)), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeAMPM,  Convert(static_cast<Controllers::Settings::Colors>(randTime)), LV_PART_MAIN | LV_STATE_DEFAULT);
      settingsController.SetPTSColorBar(static_cast<Controllers::Settings::Colors>(randBar));
      lv_obj_set_style_bg_color(sidebar,  Convert(static_cast<Controllers::Settings::Colors>(randBar)), LV_PART_MAIN | LV_STATE_DEFAULT);
      settingsController.SetPTSColorBG(static_cast<Controllers::Settings::Colors>(randBG));
      lv_obj_set_style_bg_color(timebar,  Convert(static_cast<Controllers::Settings::Colors>(randBG)), LV_PART_MAIN | LV_STATE_DEFAULT);
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
