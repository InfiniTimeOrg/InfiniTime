#include "displayapp/screens/settings/SettingTheme.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void ReloadButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
      auto* screen = static_cast<SettingTheme*>(obj->user_data);
      screen->OnReloadButtonClicked();
    }
  }
}

SettingTheme::SettingTheme(Pinetime::Applications::DisplayApp* app) : app {app} {

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 10, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 50);
  lv_cont_set_layout(container1, LV_LAYOUT_PRETTY_TOP);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::text_header);
  lv_label_set_text_static(title, "Theme");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::icon);
  lv_label_set_text_static(icon, Symbols::paintbrush);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  // Status label showing whether built-in or custom theme is used
  statusLabel = lv_label_create(container1, nullptr);
  lv_label_set_long_mode(statusLabel, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(statusLabel, LV_HOR_RES - 40);
  lv_label_set_text_static(statusLabel, "Checking...");
  lv_obj_set_style_local_text_color(statusLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  // Reload button
  reloadButton = lv_btn_create(container1, nullptr);
  lv_obj_set_width(reloadButton, LV_HOR_RES - 40);
  lv_obj_set_height(reloadButton, 50);
  lv_obj_set_style_local_radius(reloadButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_bg_color(reloadButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x38, 0x38, 0x38));
  lv_obj_set_style_local_bg_opa(reloadButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_text_color(reloadButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_pad_all(reloadButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 10);

  lv_obj_t* btnLabel = lv_label_create(reloadButton, nullptr);
  lv_label_set_text_static(btnLabel, "Reload Theme");
  lv_label_set_align(btnLabel, LV_LABEL_ALIGN_CENTER);

  reloadButton->user_data = this;
  lv_obj_set_event_cb(reloadButton, ReloadButtonEventHandler);

  // Update status to show current theme
  UpdateThemeStatus();
}

SettingTheme::~SettingTheme() {
  lv_obj_clean(lv_scr_act());
}

void SettingTheme::UpdateThemeStatus() {
  if (statusLabel == nullptr) {
    return;
  }

  if (lv_pinetime_theme_is_custom_loaded()) {
    lv_label_set_text_static(statusLabel, "Custom theme loaded");
    lv_obj_set_style_local_text_color(statusLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  } else {
    lv_label_set_text_static(statusLabel, "Using built-in colors");
    lv_obj_set_style_local_text_color(statusLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  }
}

void SettingTheme::OnReloadButtonClicked() {
  // Reload the theme configuration from the filesystem
  lv_pinetime_theme_reload_config();

  // Close this screen - when the user returns, it will be recreated with the new theme applied
  app->StartApp(Apps::Settings, DisplayApp::FullRefreshDirections::Up);
}
