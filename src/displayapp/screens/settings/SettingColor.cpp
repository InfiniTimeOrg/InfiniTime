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

  colorScheme = settingsController.getColorScheme();

  elementColor = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_state(elementColor, PT_STATE_PRIMARY);
  lv_obj_set_style_local_radius(elementColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 15);
  lv_obj_set_size(elementColor, 220, 60);
  lv_obj_align(elementColor, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);

  elementTint = lv_bar_create(lv_scr_act(), nullptr);
  lv_bar_set_range(elementTint, 0, 9);
  lv_bar_set_value(elementTint, getCurrentTint(), LV_ANIM_OFF);
  lv_obj_set_state(elementTint, PT_STATE_PRIMARY);
  lv_obj_set_style_local_radius(elementTint, LV_BAR_PART_BG, LV_STATE_DEFAULT, 15);
  lv_obj_set_size(elementTint, 220, 60);
  lv_obj_align(elementTint, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  // backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  // lv_obj_set_click(backgroundLabel, true);
  // lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  // lv_obj_set_size(backgroundLabel, 240, 240);
  // lv_obj_set_pos(backgroundLabel, 0, 0);
  // lv_label_set_text(backgroundLabel, "");

  btnNextColor = lv_btn_create(lv_scr_act(), nullptr);
  btnNextColor->user_data = this;
  lv_obj_set_size(btnNextColor, 110, 60);
  lv_obj_align(btnNextColor, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -10, -80);
  lv_obj_set_style_local_bg_opa(btnNextColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_style_local_value_str(btnNextColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "    >");
  lv_obj_set_event_cb(btnNextColor, event_handler);

  btnPrevColor = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevColor->user_data = this;
  lv_obj_set_size(btnPrevColor, 110, 60);
  lv_obj_align(btnPrevColor, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 10, -80);
  lv_obj_set_style_local_bg_opa(btnPrevColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_style_local_value_str(btnPrevColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "<    ");
  lv_obj_set_event_cb(btnPrevColor, event_handler);

  labelColor = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelColor, "Prim Color");
  lv_obj_align(labelColor, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);
  lv_label_set_align(labelColor, LV_LABEL_ALIGN_CENTER);


  btnNextTint = lv_btn_create(lv_scr_act(), nullptr);
  btnNextTint->user_data = this;
  lv_obj_set_size(btnNextTint, 110, 60);
  lv_obj_align(btnNextTint, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -10, 0);
  lv_obj_set_style_local_bg_opa(btnNextTint, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_style_local_value_str(btnNextTint, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "    >");
  lv_obj_set_event_cb(btnNextTint, event_handler);

  btnPrevTint = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevTint->user_data = this;
  lv_obj_set_size(btnPrevTint, 110, 60);
  lv_obj_align(btnPrevTint, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 10, 0);
  lv_obj_set_style_local_bg_opa(btnPrevTint, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_style_local_value_str(btnPrevTint, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "<    ");
  lv_obj_set_event_cb(btnPrevTint, event_handler);

  labelTint = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelTint, "Tint");
  lv_obj_align(labelTint, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_label_set_align(labelTint, LV_LABEL_ALIGN_CENTER);

  btnReset = lv_btn_create(lv_scr_act(), nullptr);
  btnReset->user_data = this;
  lv_obj_set_size(btnReset, 60, 60);
  lv_obj_align(btnReset, lv_scr_act(), LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_local_value_str(btnReset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Rst");
  lv_obj_set_event_cb(btnReset, event_handler);

  btnNextPage = lv_btn_create(lv_scr_act(), nullptr);
  btnNextPage->user_data = this;
  lv_obj_set_size(btnNextPage, 70, 60);
  lv_obj_align(btnNextPage, lv_scr_act(), LV_ALIGN_CENTER, 70, 80);
  lv_obj_set_style_local_value_str(btnNextPage, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Next");
  lv_obj_set_event_cb(btnNextPage, event_handler);
  
  btnPrevPage = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevPage->user_data = this;
  lv_obj_set_size(btnPrevPage, 70, 60);
  lv_obj_align(btnPrevPage, lv_scr_act(), LV_ALIGN_CENTER, -70, 80);
  lv_obj_set_style_local_value_str(btnPrevPage, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Prev");
  lv_obj_set_event_cb(btnPrevPage, event_handler);

}

SettingColor::~SettingColor() {
  settingsController.setColorScheme(colorScheme);
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}


void SettingColor::updateUI() {
  lv_obj_set_style_local_bg_color(elementColor, LV_LABEL_PART_MAIN, PT_STATE_PRIMARY, Convert(colorScheme.primary, colorScheme.primaryTint));
  lv_obj_set_style_local_bg_color(elementColor, LV_LABEL_PART_MAIN, PT_STATE_SECONDARY, Convert(colorScheme.secondary, colorScheme.secondaryTint));
  lv_obj_set_style_local_bg_color(elementColor, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(colorScheme.surface, colorScheme.surfaceTint));

  lv_obj_set_style_local_bg_color(elementTint, LV_BAR_PART_INDIC, PT_STATE_PRIMARY, Convert(colorScheme.primary, colorScheme.primaryTint));
  lv_obj_set_style_local_bg_color(elementTint, LV_BAR_PART_INDIC, PT_STATE_SECONDARY, Convert(colorScheme.secondary, colorScheme.secondaryTint));
  lv_obj_set_style_local_bg_color(elementTint, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Convert(colorScheme.surface, colorScheme.surfaceTint));

  lv_bar_set_value(elementTint, getCurrentTint(), LV_ANIM_OFF);

  lv_obj_set_style_local_bg_color(btnReset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(colorScheme.surface, colorScheme.surfaceTint));
  lv_obj_set_style_local_bg_color(btnNextPage, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(colorScheme.surface, colorScheme.surfaceTint));
  lv_obj_set_style_local_bg_color(btnPrevPage, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(colorScheme.surface, colorScheme.surfaceTint));

}

void SettingColor::setPage(Page newPage) {
  switch (newPage) {
    case Page::Primary:
      lv_obj_set_state(elementColor, PT_STATE_PRIMARY);
      lv_obj_set_state(elementTint, PT_STATE_PRIMARY);
      lv_label_set_text_static(labelColor, "Prim Color");
      break;
    case Page::Secondary:
      lv_obj_set_state(elementColor, PT_STATE_SECONDARY);
      lv_obj_set_state(elementTint, PT_STATE_SECONDARY);
      lv_label_set_text_static(labelColor, "Sec Color");
      break;
    case Page::Surface:
      lv_obj_set_state(elementColor, LV_STATE_DEFAULT);
      lv_obj_set_state(elementTint, LV_STATE_DEFAULT);
      lv_label_set_text_static(labelColor, "Btn Color");
      break;
    case Page::Background:
      lv_obj_set_state(elementColor, LV_STATE_DEFAULT);
      lv_obj_set_state(elementTint, LV_STATE_DEFAULT);
      lv_label_set_text_static(labelColor, "BG Color");
      break;    
  }

    lv_obj_realign(labelColor);

}

void SettingColor::UpdateSelected(lv_obj_t* object, lv_event_t event) {

  if (event == LV_EVENT_CLICKED) {
    if (object == btnNextColor) {
      nextColor();
    }
    if (object == btnPrevColor) {
      prevColor();
    }
    if (object == btnNextTint) {
      nextTint();
    }
    if (object == btnPrevTint) {
      prevTint();
    }
    if (object == btnNextPage) {
      switch(currentPage) {
        case Page::Primary:
          currentPage = Page::Secondary;
          break;
        case Page::Secondary:
          currentPage = Page::Surface;
          break;
        case Page::Surface:
          currentPage = Page::Background;
          break;
        case Page::Background:
          currentPage = Page::Primary;
      }

      setPage(currentPage);
    }
    if (object == btnPrevPage) {
      switch(currentPage) {
        case Page::Primary:
          currentPage = Page::Background;
          break;
        case Page::Secondary:
          currentPage = Page::Primary;
          break;
        case Page::Surface:
          currentPage = Page::Secondary;
          break;
        case Page::Background:
          currentPage = Page::Surface;
      }

      setPage(currentPage);
    }
     if (object == btnReset) {
      colorScheme.primary = Controllers::Settings::Colors::Orange;
      colorScheme.primaryTint = 4;
      colorScheme.secondary = Controllers::Settings::Colors::Cyan;
      colorScheme.secondaryTint = 7;
      colorScheme.surface = Controllers::Settings::Colors::Grey;
      colorScheme.surfaceTint = 0;
      colorScheme.background = Controllers::Settings::Colors::Black;
      colorScheme.backgroundTint = 0;
    }

    updateUI();
  }
}

Pinetime::Controllers::Settings::Colors Pinetime::Applications::Screens::SettingColor::getCurrentColor() {
  switch (currentPage) {
  case Page::Primary:
    return colorScheme.primary;
    break;
  case Page::Secondary:
    return colorScheme.secondary;
    break;
  case Page::Surface:
    return colorScheme.surface;
    break;
  case Page::Background:
    return colorScheme.background;
    break;
  default:
    return colorScheme.background;
  }
}

void Pinetime::Applications::Screens::SettingColor::setCurrentColor(Pinetime::Controllers::Settings::Colors color) {
  switch (currentPage) {
  case Page::Primary:
    colorScheme.primary = color;
    break;
  case Page::Secondary:
    colorScheme.secondary = color;
    break;
  case Page::Surface:
    colorScheme.surface = color;
    break;
  case Page::Background:
    colorScheme.background = color;
    break;
  }
}

void Pinetime::Applications::Screens::SettingColor::nextColor() {
  auto colorAsInt = static_cast<uint8_t>(getCurrentColor());
  setCurrentColor(static_cast<Controllers::Settings::Colors>((colorAsInt + 1) % 21));
  setCurrentTint(4);
}

void Pinetime::Applications::Screens::SettingColor::prevColor() {
  auto colorAsInt = static_cast<uint8_t>(getCurrentColor());
  if (colorAsInt == 0) {
    setCurrentColor(Pinetime::Controllers::Settings::Colors::White);
  } else {
    setCurrentColor(static_cast<Controllers::Settings::Colors>((colorAsInt - 1) % 21));
  }
  setCurrentTint(4);
}

uint8_t Pinetime::Applications::Screens::SettingColor::getCurrentTint() {
  switch (currentPage) {
  case Page::Primary:
    return colorScheme.primaryTint;
    break;
  case Page::Secondary:
    return colorScheme.secondaryTint;
    break;
  case Page::Surface:
    return colorScheme.surfaceTint;
    break;
  case Page::Background:
    return colorScheme.backgroundTint;
    break;
  default:
    return colorScheme.backgroundTint;
  }
}

void Pinetime::Applications::Screens::SettingColor::setCurrentTint(uint8_t tint) {
  if (tint > 9) {
    return;
  }
  
  switch (currentPage) {
  case Page::Primary:
    colorScheme.primaryTint = tint;
    break;
  case Page::Secondary:
    colorScheme.secondaryTint = tint;
    break;
  case Page::Surface:
    colorScheme.surfaceTint = tint;
    break;
  case Page::Background:
    colorScheme.backgroundTint = tint;
    break;
  }
}

void Pinetime::Applications::Screens::SettingColor::nextTint() {
  uint8_t currentTint = getCurrentTint();
  if (currentTint < 9) {
    setCurrentTint(currentTint + 1);
  }
}

void Pinetime::Applications::Screens::SettingColor::prevTint() {
  uint8_t currentTint = getCurrentTint();
  if (currentTint > 0) {
    setCurrentTint(currentTint - 1);
  }
}

