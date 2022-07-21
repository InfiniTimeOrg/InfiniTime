#include "displayapp/widgets/StatusIcons.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Widgets;

StatusIcons::StatusIcons(Controllers::Battery& batteryController, Controllers::Ble& bleController)
  : batteryController {batteryController}, bleController {bleController} {
}

void StatusIcons::Align() {
  lv_obj_t* lastIcon = batteryIcon.GetObject();

  for (auto& icon : icons) {
    if (!lv_obj_get_hidden(icon)) {
      lv_obj_align(icon, lastIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);
      lastIcon = icon;
    }
  }
}

void StatusIcons::Create() {
  batteryIcon.Create(lv_scr_act());
  lv_obj_align(batteryIcon.GetObject(), lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  icons[Icons::BatteryPlug] = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icons[Icons::BatteryPlug], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF0000));
  lv_label_set_text_static(icons[Icons::BatteryPlug], Screens::Symbols::plug);

  icons[Icons::BleIcon] = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icons[Icons::BleIcon], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0082FC));
  lv_label_set_text_static(icons[Icons::BleIcon], Screens::Symbols::bluetooth);

  Align();
}

void StatusIcons::Update() {
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    lv_obj_set_hidden(icons[Icons::BatteryPlug], !powerPresent.Get());
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    batteryIcon.SetBatteryPercentage(batteryPercent);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_obj_set_hidden(icons[Icons::BleIcon], !bleState.Get());
  }

  Align();
}
