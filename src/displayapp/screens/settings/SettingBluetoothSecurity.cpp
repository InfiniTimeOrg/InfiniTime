#include "displayapp/screens/settings/SettingBluetoothSecurity.h"
#include <lvgl/lvgl.h>
#include <hal/nrf_rtc.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/Messages.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void SecurityEventHandler(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
      auto* screen = static_cast<SettingBluetoothSecurity*>(obj->user_data);
      screen->ToggleSecurity();
    }
  }

  void PairingEventHandler(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
      auto* screen = static_cast<SettingBluetoothSecurity*>(obj->user_data);
      screen->TogglePairing();
    }
  }
}

SettingBluetoothSecurity::SettingBluetoothSecurity(Pinetime::Applications::DisplayApp* app,
                                                   Pinetime::Controllers::Settings& settingsController)
  : app {app}, settings {settingsController} {
  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 10, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 20);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "BT security");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::bluetooth);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  secured = settings.GetBleSecured();
  securityCheckbox = lv_checkbox_create(container1, nullptr);
  lv_checkbox_set_text(securityCheckbox, "Secured");
  lv_checkbox_set_checked(securityCheckbox, secured);
  lv_obj_add_state(securityCheckbox, LV_STATE_DEFAULT);
  securityCheckbox->user_data = this;
  lv_obj_set_event_cb(securityCheckbox, SecurityEventHandler);

  pairingCheckbox = lv_checkbox_create(container1, nullptr);
  lv_checkbox_set_text(pairingCheckbox, "Pairable");
  lv_obj_add_state(pairingCheckbox, LV_STATE_DEFAULT);
  pairingCheckbox->user_data = this;
  lv_checkbox_set_checked(pairingCheckbox, settings.GetBlePairingAllowed() && settings.GetBleRadioEnabled());
  if (settings.GetBleSecured() && settings.GetBleRadioEnabled()) {
    lv_obj_set_event_cb(pairingCheckbox, PairingEventHandler);
  } else {
    lv_checkbox_set_disabled(pairingCheckbox);
  }

  statusLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(statusLabel, "");
  lv_label_set_recolor(statusLabel, true);
  lv_obj_set_auto_realign(statusLabel, true);
  lv_obj_align(statusLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 176);
}

SettingBluetoothSecurity::~SettingBluetoothSecurity() {
  lv_obj_clean(lv_scr_act());
  settings.SetBlePairingAllowed(false);
  if (secured != settings.GetBleSecured()) {
    settings.SetBleSecured(secured);
    settings.SaveSettings();
    settings.SetBleRadioEnabled(false);
    this->app->PushMessage(Pinetime::Applications::Display::Messages::BleRadioEnableToggle);
    vTaskDelay(pdMS_TO_TICKS(1000));
    NVIC_SystemReset();
  } else if (settings.GetBleSecured() && settings.GetBleRadioEnabled()) {
    this->app->PushMessage(Pinetime::Applications::Display::Messages::BleAllowPairingToggle);
  }
}

void SettingBluetoothSecurity::ToggleSecurity() {
  secured = !secured;
  lv_checkbox_set_checked(securityCheckbox, secured);
  UpdateRebootMessage();
}

void SettingBluetoothSecurity::UpdateRebootMessage() {
  if (secured != settings.GetBleSecured()) {
    lv_label_set_text_static(statusLabel, "#ffa500 Reboot needed.#");
  } else {
    lv_label_set_text_static(statusLabel, "");
  }
}

void SettingBluetoothSecurity::TogglePairing() {
  settings.SetBlePairingAllowed(!settings.GetBlePairingAllowed());
  lv_checkbox_set_checked(pairingCheckbox, settings.GetBlePairingAllowed());
  if (settings.GetBlePairingAllowed()) {
    lv_label_set_text_static(statusLabel, "#00ff00 Pair NOW!#");
  } else {
    UpdateRebootMessage();
  }
  this->app->PushMessage(Pinetime::Applications::Display::Messages::BleAllowPairingToggle);
}
