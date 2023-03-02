#include "displayapp/widgets/StatusIcons.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Widgets;

StatusIcons::StatusIcons(const Controllers::Battery& batteryController, const Controllers::Ble& bleController)
  : batteryIcon(true), batteryController {batteryController}, bleController {bleController} {
}

void StatusIcons::Create() {
  container = lv_cont_create(lv_scr_act(), nullptr);
  lv_cont_set_layout(container, LV_LAYOUT_ROW_TOP);
  lv_cont_set_fit(container, LV_FIT_TIGHT);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

  bleIcon = lv_label_create(container, nullptr);
  lv_label_set_text_static(bleIcon, Screens::Symbols::bluetooth);

  batteryPlug = lv_label_create(container, nullptr);
  lv_label_set_text_static(batteryPlug, Screens::Symbols::plug);

  batteryIcon.Create(container);

  lv_obj_align(container, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
}

void StatusIcons::Update() {
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    lv_obj_set_hidden(batteryPlug, !powerPresent.Get());
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    batteryIcon.SetBatteryPercentage(batteryPercent);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_obj_set_hidden(bleIcon, !bleState.Get());
  }

  lv_obj_realign(container);
}
