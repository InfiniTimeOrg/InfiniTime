#include "SettingColor.h"
#include <lvgl/lvgl.h>
#include <displayapp/Colors.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/lv_pinetime_theme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t* obj, lv_event_t event) {
    SettingColor* screen = static_cast<SettingColor*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingColor::SettingColor(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController} {
  primaryColor = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(primaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetColorList()));
  lv_obj_set_style_local_bg_opa(primaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, settingsController.GetOpacity());
  lv_obj_set_style_local_radius(primaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 15);
  lv_obj_set_size(primaryColor, 220, 60);
  lv_obj_align(primaryColor, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);

  secondaryColor = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(secondaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetColorTile()));
  lv_obj_set_style_local_bg_opa(secondaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, settingsController.GetOpacity());
  lv_obj_set_style_local_radius(secondaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 15);
  lv_obj_set_size(secondaryColor, 220, 60);
  lv_obj_align(secondaryColor, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  // backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  // lv_obj_set_click(backgroundLabel, true);
  // lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  // lv_obj_set_size(backgroundLabel, 240, 240);
  // lv_obj_set_pos(backgroundLabel, 0, 0);
  // lv_label_set_text(backgroundLabel, "");

  btnNextPrimary = lv_btn_create(lv_scr_act(), nullptr);
  btnNextPrimary->user_data = this;
  lv_obj_set_size(btnNextPrimary, 110, 60);
  lv_obj_align(btnNextPrimary, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -10, -80);
  //lv_obj_set_style_local_bg_color(btnNextPrimary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetColorList()));
  lv_obj_set_style_local_bg_opa(btnNextPrimary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  //lv_obj_set_style_local_radius(btnNextPrimary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_value_str(btnNextPrimary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "    >");
  lv_obj_set_event_cb(btnNextPrimary, event_handler);

  btnPrevPrimary = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevPrimary->user_data = this;
  lv_obj_set_size(btnPrevPrimary, 110, 60);
  lv_obj_align(btnPrevPrimary, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 10, -80);
  //lv_obj_set_style_local_bg_color(btnPrevPrimary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetColorList()));
  lv_obj_set_style_local_bg_opa(btnPrevPrimary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  //lv_obj_set_style_local_radius(btnPrevPrimary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_value_str(btnPrevPrimary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "<    ");
  lv_obj_set_event_cb(btnPrevPrimary, event_handler);

  labelPrimary = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelPrimary, "Primary");
  lv_obj_align(labelPrimary, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);


  btnNextSecondary = lv_btn_create(lv_scr_act(), nullptr);
  btnNextSecondary->user_data = this;
  lv_obj_set_size(btnNextSecondary, 110, 60);
  lv_obj_align(btnNextSecondary, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -10, 0);
  //lv_obj_set_style_local_bg_color(btnNextSecondary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetColorTile()));
  lv_obj_set_style_local_bg_opa(btnNextSecondary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  //lv_obj_set_style_local_radius(btnNextSecondary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_value_str(btnNextSecondary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "    >");
  lv_obj_set_event_cb(btnNextSecondary, event_handler);

  btnPrevSecondary = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevSecondary->user_data = this;
  lv_obj_set_size(btnPrevSecondary, 110, 60);
  lv_obj_align(btnPrevSecondary, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 10, 0);
  //lv_obj_set_style_local_bg_color(btnPrevSecondary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetColorTile()));
  lv_obj_set_style_local_bg_opa(btnPrevSecondary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  //lv_obj_set_style_local_radius(btnPrevSecondary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_value_str(btnPrevSecondary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "<    ");
  lv_obj_set_event_cb(btnPrevSecondary, event_handler);

  labelSecondary = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelSecondary, "Secondary");
  lv_obj_align(labelSecondary, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  btnOpacity = lv_btn_create(lv_scr_act(), nullptr);
  btnOpacity->user_data = this;
  lv_obj_set_size(btnOpacity, 150, 60);
  lv_obj_align(btnOpacity, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 10, 80);
  lv_obj_set_style_local_bg_opa(btnOpacity, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnOpacity, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Opacity");
  lv_obj_set_event_cb(btnOpacity, event_handler);

  btnReset = lv_btn_create(lv_scr_act(), nullptr);
  btnReset->user_data = this;
  lv_obj_set_size(btnReset, 60, 60);
  lv_obj_align(btnReset, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -10, 80);
  lv_obj_set_style_local_bg_opa(btnReset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnReset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Rst");
  lv_obj_set_event_cb(btnReset, event_handler);

}

SettingColor::~SettingColor() {
  pt_update_theme(settingsController.getPrimaryColor(), settingsController.getSecondaryColor(), settingsController.getSurfaceColor(), settingsController.getBackgroundColor());
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingColor::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  auto valueList = settingsController.GetColorList();
  auto valueTile = settingsController.GetColorTile();
  auto valueOpacity = settingsController.GetOpacity();

  if (event == LV_EVENT_CLICKED) {
    if (object == btnNextPrimary) {
      valueList = GetNext(valueList);
      if(valueList == Controllers::Settings::Colors::White)
        valueList = GetNext(valueList);
      if(valueList == Controllers::Settings::Colors::Black)
        valueList = GetNext(valueList);
      settingsController.SetColorList(valueList);
      lv_obj_set_style_local_bg_color(primaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueList));
      settingsController.setPrimaryColor(Convert(valueList));
      //lv_obj_set_style_local_bg_color(btnNextPrimary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueList));
      //lv_obj_set_style_local_bg_color(btnPrevPrimary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueList));
    }
    if (object == btnPrevPrimary) {
      valueList = GetPrevious(valueList);
      if(valueList == Controllers::Settings::Colors::White)
        valueList = GetPrevious(valueList);
      if(valueList == Controllers::Settings::Colors::Black)
        valueList = GetPrevious(valueList);
      settingsController.SetColorList(valueList);
      lv_obj_set_style_local_bg_color(primaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueList));
      settingsController.setPrimaryColor(Convert(valueList));
      //lv_obj_set_style_local_bg_color(btnNextPrimary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueList));
      //lv_obj_set_style_local_bg_color(btnPrevPrimary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueList));
    }
    if (object == btnNextSecondary) {
      valueTile = GetNext(valueTile);
      if(valueTile == Controllers::Settings::Colors::White)
        valueTile = GetNext(valueTile);
      if(valueTile == Controllers::Settings::Colors::Black)
        valueTile = GetNext(valueTile);
      settingsController.SetColorTile(valueTile);
      lv_obj_set_style_local_bg_color(secondaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTile));
      settingsController.setSecondaryColor(Convert(valueTile));
      //lv_obj_set_style_local_bg_color(btnNextSecondary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTile));
      //lv_obj_set_style_local_bg_color(btnPrevSecondary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTile));
    }
    if (object == btnPrevSecondary) {
      valueTile = GetPrevious(valueTile);
      if(valueTile == Controllers::Settings::Colors::White)
        valueTile = GetPrevious(valueTile);
      if(valueTile == Controllers::Settings::Colors::Black)
        valueTile = GetPrevious(valueTile);
      settingsController.SetColorTile(valueTile);
      lv_obj_set_style_local_bg_color(secondaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTile));
      settingsController.setSecondaryColor(Convert(valueTile));
      //lv_obj_set_style_local_bg_color(btnNextSecondary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTile));
      //lv_obj_set_style_local_bg_color(btnPrevSecondary, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTile));
    }
    if (object == btnOpacity) {
      valueOpacity = valueOpacity + 51; 
      if (valueOpacity > 255)
        valueOpacity = 51;
      settingsController.SetOpacity(valueOpacity);
      lv_obj_set_style_local_bg_opa(secondaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, valueOpacity);
      lv_obj_set_style_local_bg_opa(primaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, valueOpacity);
    }
     if (object == btnReset) {
      settingsController.SetColorList(Controllers::Settings::Colors::Orange);
      lv_obj_set_style_local_bg_color(primaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Orange));
      settingsController.SetColorTile(Controllers::Settings::Colors::Cyan);
      lv_obj_set_style_local_bg_color(secondaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Cyan));
      settingsController.SetOpacity(255);
      lv_obj_set_style_local_bg_opa(secondaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 255);
      lv_obj_set_style_local_bg_opa(primaryColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 255);
      settingsController.setPrimaryColor(Convert(Controllers::Settings::Colors::Orange));
      settingsController.setSecondaryColor(Convert(Controllers::Settings::Colors::Cyan));
    }
  }
}

Pinetime::Controllers::Settings::Colors SettingColor::GetNext(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors nextColor;
  if (colorAsInt < 16) {
    nextColor = static_cast<Controllers::Settings::Colors>(colorAsInt + 1);
  } else {
    nextColor = static_cast<Controllers::Settings::Colors>(0);
  }
  return nextColor;
}

Pinetime::Controllers::Settings::Colors SettingColor::GetPrevious(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors prevColor;

  if (colorAsInt > 0) {
    prevColor = static_cast<Controllers::Settings::Colors>(colorAsInt - 1);
  } else {
    prevColor = static_cast<Controllers::Settings::Colors>(16);
  }
  return prevColor;
}