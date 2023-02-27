#include "displayapp/screens/settings/SettingPeriodicAlert.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingPeriodicAlert*>(obj->user_data);
    screen->OnButtonEvent(obj, event);
  }

  void valueChangedHandler(void* userdata) {
    auto* screen = static_cast<SettingPeriodicAlert*>(userdata);
    screen->OnValueChanged();
  }
}

SettingPeriodicAlert::SettingPeriodicAlert(Pinetime::Controllers::Settings& settingsController,
                                           Pinetime::Controllers::PeriodicAlertController& controller)
: controller{controller}
, settingsController{settingsController} {
  static constexpr lv_color_t bgColor = Colors::bgAlt;
  static constexpr uint8_t h_offset = 55;
  static constexpr uint8_t v_offset = 40;

  enableSwitch = lv_switch_create(lv_scr_act(), nullptr);
  enableSwitch->user_data = this;
  lv_obj_set_event_cb(enableSwitch, btnEventHandler);
  lv_obj_set_size(enableSwitch, 100, 50);
  lv_obj_align(enableSwitch, lv_scr_act(), LV_ALIGN_CENTER, -h_offset, v_offset);
  lv_obj_set_style_local_bg_color(enableSwitch, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, bgColor);

  minuteCounter.Create();
  lv_obj_align(minuteCounter.GetObject(), lv_scr_act(), LV_ALIGN_CENTER, h_offset, 0);
  minuteCounter.SetValueChangedEventCallback(this, valueChangedHandler);
  if (settingsController.GetPeriodicAlertEnabled()) {
    minuteCounter.SetValue(settingsController.GetPeriodicAlertMinute());
    lv_switch_on(enableSwitch, LV_ANIM_OFF);
  } else {
    lv_switch_off(enableSwitch, LV_ANIM_OFF);
  }

  helpText = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(helpText, "Alerts on\nthe set\nminute\nevery hour");
  lv_obj_align(helpText, lv_scr_act(), LV_ALIGN_CENTER, -h_offset, -v_offset);
}

SettingPeriodicAlert::~SettingPeriodicAlert() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingPeriodicAlert::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  (void) obj;
  (void) event;
  Update();
}

void SettingPeriodicAlert::OnValueChanged() {
  lv_switch_off(enableSwitch, LV_ANIM_ON);
  Update();
}

void SettingPeriodicAlert::Update() {
  if (lv_switch_get_state(enableSwitch)) {
    controller.SetPeriodicAlertMinute(minuteCounter.GetValue());
    controller.Enable();
  } else {
    controller.Disable();
  }
}
