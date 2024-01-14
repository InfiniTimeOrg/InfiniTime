#include "displayapp/widgets/StatusIcons.h"
#include "displayapp/screens/Symbols.h"
#include "components/alarm/AlarmController.h"

using namespace Pinetime::Applications::Widgets;

StatusIcons::StatusIcons(const Controllers::Battery& batteryController,
                         const Controllers::Ble& bleController,
                         const Controllers::AlarmController& alarmController,
                         const Controllers::Timer& timer)
  : batteryIcon(true),
    batteryController {batteryController},
    bleController {bleController},
    alarmController {alarmController},
    timer {timer} {
}

void StatusIcons::Create() {
  timerIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(timerIcon, Screens::Symbols::hourGlass);
  lv_obj_set_style_local_text_color(timerIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_obj_align(timerIcon, lv_scr_act(), LV_ALIGN_IN_TOP_MID, -38, 0);

  timeRemaining = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(timeRemaining, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_label_set_text(timeRemaining, "00:00");
  lv_obj_align(timeRemaining, nullptr, LV_ALIGN_IN_TOP_MID, 4, 0);

  container = lv_cont_create(lv_scr_act(), nullptr);
  lv_cont_set_layout(container, LV_LAYOUT_ROW_TOP);
  lv_cont_set_fit(container, LV_FIT_TIGHT);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

  bleIcon = lv_label_create(container, nullptr);
  lv_label_set_text_static(bleIcon, Screens::Symbols::bluetooth);

  batteryPlug = lv_label_create(container, nullptr);
  lv_label_set_text_static(batteryPlug, Screens::Symbols::plug);

  alarmIcon = lv_label_create(container, nullptr);
  lv_label_set_text_static(alarmIcon, Screens::Symbols::bell);

  batteryIcon.Create(container);

  lv_obj_align(container, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
}

void StatusIcons::Update() {
  if (timer.IsRunning()) {
    auto secondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimeRemaining());

    uint8_t minutes = secondsRemaining.count() / 60;
    uint8_t seconds = secondsRemaining.count() % 60;
    lv_label_set_text_fmt(timeRemaining, "%02d:%02d", minutes, seconds);

    lv_obj_set_hidden(timeRemaining, false);
    lv_obj_set_hidden(timerIcon, false);
  } else {
    lv_obj_set_hidden(timeRemaining, true);
    lv_obj_set_hidden(timerIcon, true);
  }

  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    lv_obj_set_hidden(batteryPlug, !powerPresent.Get());
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    batteryIcon.SetBatteryPercentage(batteryPercent);
  }

  alarmEnabled = alarmController.IsEnabled();
  if (alarmEnabled.IsUpdated()) {
    lv_obj_set_hidden(alarmIcon, !alarmEnabled.Get());
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_obj_set_hidden(bleIcon, !bleState.Get());
  }

  lv_obj_realign(container);
}
