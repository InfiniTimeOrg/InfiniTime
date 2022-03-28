#include "SettingColor.h"
#include <lvgl/lvgl.h>
#include <displayapp/Colors.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t* obj, lv_event_t event) {
    SettingColor* screen = static_cast<SettingColor*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingColor::SettingColor(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController} {
  listColor = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(listColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetColorList()));
  lv_obj_set_style_local_bg_opa(listColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, settingsController.GetOpacity());
  lv_obj_set_style_local_radius(listColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 15);
  lv_obj_set_size(listColor, 220, 60);
  lv_obj_align(listColor, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);

  tileColor = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(tileColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetColorTile()));
  lv_obj_set_style_local_bg_opa(tileColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, settingsController.GetOpacity());
  lv_obj_set_style_local_radius(tileColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 15);
  lv_obj_set_size(tileColor, 220, 60);
  lv_obj_align(tileColor, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");

  btnNextList = lv_btn_create(lv_scr_act(), nullptr);
  btnNextList->user_data = this;
  lv_obj_set_size(btnNextList, 110, 60);
  lv_obj_align(btnNextList, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -10, -80);
  //lv_obj_set_style_local_bg_color(btnNextList, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetColorList()));
  lv_obj_set_style_local_bg_opa(btnNextList, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  //lv_obj_set_style_local_radius(btnNextList, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_value_str(btnNextList, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "    >");
  lv_obj_set_event_cb(btnNextList, event_handler);

  btnPrevList = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevList->user_data = this;
  lv_obj_set_size(btnPrevList, 110, 60);
  lv_obj_align(btnPrevList, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 10, -80);
  //lv_obj_set_style_local_bg_color(btnPrevList, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetColorList()));
  lv_obj_set_style_local_bg_opa(btnPrevList, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  //lv_obj_set_style_local_radius(btnPrevList, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_value_str(btnPrevList, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "<    ");
  lv_obj_set_event_cb(btnPrevList, event_handler);

  labelList = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelList, "Settings");
  lv_obj_align(labelList, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);

  btnNextTile = lv_btn_create(lv_scr_act(), nullptr);
  btnNextTile->user_data = this;
  lv_obj_set_size(btnNextTile, 110, 60);
  lv_obj_align(btnNextTile, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -10, 0);
  //lv_obj_set_style_local_bg_color(btnNextTile, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetColorTile()));
  lv_obj_set_style_local_bg_opa(btnNextTile, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  //lv_obj_set_style_local_radius(btnNextTile, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_value_str(btnNextTile, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "    >");
  lv_obj_set_event_cb(btnNextTile, event_handler);

  btnPrevTile = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevTile->user_data = this;
  lv_obj_set_size(btnPrevTile, 110, 60);
  lv_obj_align(btnPrevTile, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 10, 0);
  //lv_obj_set_style_local_bg_color(btnPrevTile, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetColorTile()));
  lv_obj_set_style_local_bg_opa(btnPrevTile, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  //lv_obj_set_style_local_radius(btnPrevTile, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_value_str(btnPrevTile, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "<    ");
  lv_obj_set_event_cb(btnPrevTile, event_handler);

  labelTile = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelTile, "Apps");
  lv_obj_align(labelTile, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

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
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingColor::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  auto valueList = settingsController.GetColorList();
  auto valueTile = settingsController.GetColorTile();
  auto valueOpacity = settingsController.GetOpacity();

  if (event == LV_EVENT_CLICKED) {
    if (object == btnNextList) {
      valueList = GetNext(valueList);
      if(valueList == Controllers::Settings::Colors::White)
        valueList = GetNext(valueList);
      if(valueList == Controllers::Settings::Colors::Black)
        valueList = GetNext(valueList);
      settingsController.SetColorList(valueList);
      lv_obj_set_style_local_bg_color(listColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueList));
      //lv_obj_set_style_local_bg_color(btnNextList, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueList));
      //lv_obj_set_style_local_bg_color(btnPrevList, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueList));
    }
    if (object == btnPrevList) {
      valueList = GetPrevious(valueList);
      if(valueList == Controllers::Settings::Colors::White)
        valueList = GetPrevious(valueList);
      if(valueList == Controllers::Settings::Colors::Black)
        valueList = GetPrevious(valueList);
      settingsController.SetColorList(valueList);
      lv_obj_set_style_local_bg_color(listColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueList));
      //lv_obj_set_style_local_bg_color(btnNextList, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueList));
      //lv_obj_set_style_local_bg_color(btnPrevList, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueList));
    }
    if (object == btnNextTile) {
      valueTile = GetNext(valueTile);
      if(valueTile == Controllers::Settings::Colors::White)
        valueTile = GetNext(valueTile);
      if(valueTile == Controllers::Settings::Colors::Black)
        valueTile = GetNext(valueTile);
      settingsController.SetColorTile(valueTile);
      lv_obj_set_style_local_bg_color(tileColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTile));
      //lv_obj_set_style_local_bg_color(btnNextTile, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTile));
      //lv_obj_set_style_local_bg_color(btnPrevTile, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTile));
    }
    if (object == btnPrevTile) {
      valueTile = GetPrevious(valueTile);
      if(valueTile == Controllers::Settings::Colors::White)
        valueTile = GetPrevious(valueTile);
      if(valueTile == Controllers::Settings::Colors::Black)
        valueTile = GetPrevious(valueTile);
      settingsController.SetColorTile(valueTile);
      lv_obj_set_style_local_bg_color(tileColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTile));
      //lv_obj_set_style_local_bg_color(btnNextTile, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTile));
      //lv_obj_set_style_local_bg_color(btnPrevTile, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTile));
    }
    if (object == btnOpacity) {
      valueOpacity = valueOpacity + 51; 
      if (valueOpacity > 255)
        valueOpacity = 51;
      settingsController.SetOpacity(valueOpacity);
      lv_obj_set_style_local_bg_opa(tileColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, valueOpacity);
      lv_obj_set_style_local_bg_opa(listColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, valueOpacity);
    }
     if (object == btnReset) {
      settingsController.SetColorList(Controllers::Settings::Colors::Cyan);
      lv_obj_set_style_local_bg_color(listColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Cyan));
      settingsController.SetColorTile(Controllers::Settings::Colors::Cyan);
      lv_obj_set_style_local_bg_color(tileColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Cyan));
      settingsController.SetOpacity(51);
      lv_obj_set_style_local_bg_opa(tileColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 51);
      lv_obj_set_style_local_bg_opa(listColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 51);
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