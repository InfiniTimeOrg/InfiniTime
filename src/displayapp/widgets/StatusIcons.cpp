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
  container = lv_cont_create(lv_scr_act(), nullptr);
  lv_cont_set_layout(container, LV_LAYOUT_ROW_TOP);
  lv_cont_set_fit(container, LV_FIT_TIGHT);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

  timerContainer = lv_cont_create(container, nullptr);
  lv_cont_set_layout(timerContainer, LV_LAYOUT_ROW_TOP);
  lv_cont_set_fit(timerContainer, LV_FIT_TIGHT);
  lv_obj_set_style_local_pad_inner(timerContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_bg_opa(timerContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

  timerIcon = lv_label_create(timerContainer, nullptr);
  lv_label_set_text_static(timerIcon, Screens::Symbols::hourGlass);
  lv_obj_set_style_local_text_color(timerIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));

  timeRemaining = lv_label_create(timerContainer, nullptr);
  lv_obj_set_style_local_text_color(timeRemaining, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_label_set_text(timeRemaining, "00:00");

  bleIcon = lv_label_create(container, nullptr);
  lv_label_set_text_static(bleIcon, Screens::Symbols::bluetooth);

  batteryPlug = lv_label_create(container, nullptr);
  lv_label_set_text_static(batteryPlug, Screens::Symbols::plug);

  soloTimerIcon = lv_label_create(container, nullptr);
  lv_label_set_text_static(soloTimerIcon, Screens::Symbols::hourGlass);

  alarmIcon = lv_label_create(container, nullptr);
  lv_label_set_text_static(alarmIcon, Screens::Symbols::bell);

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

  alarmEnabled = alarmController.IsEnabled();
  if (alarmEnabled.IsUpdated()) {
    lv_obj_set_hidden(alarmIcon, !alarmEnabled.Get());
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_obj_set_hidden(bleIcon, !bleState.Get());
  }

  if (timer.IsRunning()) {
    uint8_t activeIconCounter = 0;

    if (!lv_obj_get_hidden(bleIcon)) {
      activeIconCounter++;
    }
    if (!lv_obj_get_hidden(batteryPlug)) {
      activeIconCounter++;
    }
    if (!lv_obj_get_hidden(alarmIcon)) {
      activeIconCounter++;
    }

    // more than 2 icons (plus the batteryIcon) make the timerContainer collide
    // with other text, so replace it with a timer icon
    if (activeIconCounter > 2) {
      lv_obj_set_hidden(soloTimerIcon, false);
      lv_obj_set_hidden(timerContainer, true);
    } else {
      auto secondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(timer.GetTimeRemaining());
      uint8_t minutes = secondsRemaining.count() / 60;
      uint8_t seconds = secondsRemaining.count() % 60;
      lv_label_set_text_fmt(timeRemaining, "%02d:%02d", minutes, seconds);

      lv_obj_set_hidden(soloTimerIcon, true);
      lv_obj_set_hidden(timerContainer, false);
    }
  } else {
    lv_obj_set_hidden(soloTimerIcon, true);
    lv_obj_set_hidden(timerContainer, true);
  }

  lv_obj_realign(container);
}
