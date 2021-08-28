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
  : Screen(app), settingsController {settingsController} {
  timebar = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorBG()));
  lv_obj_set_style_local_radius(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(timebar, 200, 240);
  lv_obj_align(timebar, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 5, 0);

  // Display the time

  timeDD1 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &open_sans_light);
  lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorTime()));
  lv_label_set_text(timeDD1, "12");
  lv_obj_align(timeDD1, timebar, LV_ALIGN_IN_TOP_MID, 5, 5);

  timeDD2 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &open_sans_light);
  lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorTime()));
  lv_label_set_text(timeDD2, "34");
  lv_obj_align(timeDD2, timebar, LV_ALIGN_IN_BOTTOM_MID, 5, -5);

  timeAMPM = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorTime()));
  lv_obj_set_style_local_text_line_space(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, -3);
  lv_label_set_text(timeAMPM, "A\nM");
  lv_obj_align(timeAMPM, timebar, LV_ALIGN_IN_BOTTOM_LEFT, 2, -20);

  // Create a 40px wide bar down the right side of the screen

  sidebar = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorBar()));
  lv_obj_set_style_local_radius(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(sidebar, 40, 240);
  lv_obj_align(sidebar, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  // Display icons

  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_label_set_text(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, sidebar, LV_ALIGN_IN_TOP_MID, 0, 2);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_label_set_text(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, sidebar, LV_ALIGN_IN_TOP_MID, 0, 25);

  // Calendar icon

  calendarOuter = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarOuter, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_obj_set_style_local_radius(calendarOuter, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarOuter, 34, 34);
  lv_obj_align(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 0);

  calendarInner = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarInner, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
  lv_obj_set_style_local_radius(calendarInner, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarInner, 27, 27);
  lv_obj_align(calendarInner, calendarOuter, LV_ALIGN_CENTER, 0, 0);

  calendarBar1 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarBar1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_obj_set_style_local_radius(calendarBar1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarBar1, 3, 12);
  lv_obj_align(calendarBar1, calendarOuter, LV_ALIGN_IN_TOP_MID, -6, -3);

  calendarBar2 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarBar2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_obj_set_style_local_radius(calendarBar2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarBar2, 3, 12);
  lv_obj_align(calendarBar2, calendarOuter, LV_ALIGN_IN_TOP_MID, 6, -3);

  calendarCrossBar1 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarCrossBar1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_obj_set_style_local_radius(calendarCrossBar1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarCrossBar1, 8, 3);
  lv_obj_align(calendarCrossBar1, calendarBar1, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  calendarCrossBar2 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarCrossBar2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_obj_set_style_local_radius(calendarCrossBar2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarCrossBar2, 8, 3);
  lv_obj_align(calendarCrossBar2, calendarBar2, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  // Display date

  dateDayOfWeek = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(dateDayOfWeek, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_label_set_text(dateDayOfWeek, "THU");
  lv_obj_align(dateDayOfWeek, sidebar, LV_ALIGN_CENTER, 0, -34);

  dateDay = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(dateDay, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_label_set_text(dateDay, "25");
  lv_obj_align(dateDay, sidebar, LV_ALIGN_CENTER, 0, 3);

  dateMonth = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(dateMonth, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_label_set_text(dateMonth, "MAR");
  lv_obj_align(dateMonth, sidebar, LV_ALIGN_CENTER, 0, 32);

  // Step count gauge
  needle_colors[0] = LV_COLOR_WHITE;
  stepGauge = lv_gauge_create(lv_scr_act(), nullptr);
  lv_gauge_set_needle_count(stepGauge, 1, needle_colors);
  lv_obj_set_size(stepGauge, 40, 40);
  lv_obj_align(stepGauge, sidebar, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_gauge_set_scale(stepGauge, 360, 11, 0);
  lv_gauge_set_angle_offset(stepGauge, 180);
  lv_gauge_set_critical_value(stepGauge, (100));
  lv_gauge_set_range(stepGauge, 0, (100));
  lv_gauge_set_value(stepGauge, 0, 0);

  lv_obj_set_style_local_pad_right(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_pad_left(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_pad_bottom(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_line_opa(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_scale_width(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_width(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_color(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_line_opa(stepGauge, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_line_width(stepGauge, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_pad_inner(stepGauge, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, 4);

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");

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
