#include "displayapp/screens/settings/SettingBluetooth.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/Messages.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void OnBluetoothDisabledEvent(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingBluetooth*>(obj->user_data);
    screen->OnBluetoothDisabled(obj, event);
  }

  void OnBluetoothEnabledEvent(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingBluetooth*>(obj->user_data);
    screen->OnBluetoothEnabled(obj, event);
  }
}

SettingBluetooth::SettingBluetooth(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController} {

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 10, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 50);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Bluetooth");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::bluetooth);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  cbEnabled = lv_checkbox_create(container1, nullptr);
  lv_checkbox_set_text(cbEnabled, " Enabled");
  cbEnabled->user_data = this;
  lv_obj_set_event_cb(cbEnabled, OnBluetoothEnabledEvent);
  SetRadioButtonStyle(cbEnabled);

  cbDisabled = lv_checkbox_create(container1, nullptr);
  lv_checkbox_set_text(cbDisabled, " Disabled");
  cbDisabled->user_data = this;
  lv_obj_set_event_cb(cbDisabled, OnBluetoothDisabledEvent);
  SetRadioButtonStyle(cbDisabled);

  if (settingsController.GetBleRadioEnabled()) {
    lv_checkbox_set_checked(cbEnabled, true);
    priorMode = true;
  } else {
    lv_checkbox_set_checked(cbDisabled, true);
    priorMode = false;
  }
}

SettingBluetooth::~SettingBluetooth() {
  lv_obj_clean(lv_scr_act());
  // Do not call SaveSettings - see src/components/settings/Settings.h
  if (priorMode != settingsController.GetBleRadioEnabled()) {
    app->PushMessage(Pinetime::Applications::Display::Messages::BleRadioEnableToggle);
  }
}

void SettingBluetooth::OnBluetoothDisabled(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    lv_checkbox_set_checked(cbEnabled, false);
    lv_checkbox_set_checked(cbDisabled, true);
    settingsController.SetBleRadioEnabled(false);
  }
}

void SettingBluetooth::OnBluetoothEnabled(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    lv_checkbox_set_checked(cbEnabled, true);
    lv_checkbox_set_checked(cbDisabled, false);
    settingsController.SetBleRadioEnabled(true);
  }
}

