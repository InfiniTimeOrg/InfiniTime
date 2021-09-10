#include "Alarm.h"
#include "Screen.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;
using Pinetime::Controllers::AlarmController;

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  Alarm* screen = static_cast<Alarm*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

Alarm::Alarm(DisplayApp* app, Controllers::AlarmController& alarmController)
  : Screen(app), running {true}, alarmController {alarmController} {

  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  alarmHours = alarmController.Hours();
  alarmMinutes = alarmController.Minutes();
  lv_label_set_text_fmt(time, "%02lu:%02lu", alarmHours, alarmMinutes);

  lv_obj_align(time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -20);

  btnHoursUp = lv_btn_create(lv_scr_act(), nullptr);
  btnHoursUp->user_data = this;
  lv_obj_set_event_cb(btnHoursUp, btnEventHandler);
  lv_obj_align(btnHoursUp, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 20, -80);
  lv_obj_set_height(btnHoursUp, 40);
  lv_obj_set_width(btnHoursUp, 60);
  txtHrUp = lv_label_create(btnHoursUp, nullptr);
  lv_label_set_text(txtHrUp, "+");

  btnHoursDown = lv_btn_create(lv_scr_act(), nullptr);
  btnHoursDown->user_data = this;
  lv_obj_set_event_cb(btnHoursDown, btnEventHandler);
  lv_obj_align(btnHoursDown, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 20, +40);
  lv_obj_set_height(btnHoursDown, 40);
  lv_obj_set_width(btnHoursDown, 60);
  txtHrDown = lv_label_create(btnHoursDown, nullptr);
  lv_label_set_text(txtHrDown, "-");

  btnMinutesUp = lv_btn_create(lv_scr_act(), nullptr);
  btnMinutesUp->user_data = this;
  lv_obj_set_event_cb(btnMinutesUp, btnEventHandler);
  lv_obj_align(btnMinutesUp, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 10, -80);
  lv_obj_set_height(btnMinutesUp, 40);
  lv_obj_set_width(btnMinutesUp, 60);
  txtMinUp = lv_label_create(btnMinutesUp, nullptr);
  lv_label_set_text(txtMinUp, "+");

  btnMinutesDown = lv_btn_create(lv_scr_act(), nullptr);
  btnMinutesDown->user_data = this;
  lv_obj_set_event_cb(btnMinutesDown, btnEventHandler);
  lv_obj_align(btnMinutesDown, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 10, +40);
  lv_obj_set_height(btnMinutesDown, 40);
  lv_obj_set_width(btnMinutesDown, 60);
  txtMinDown = lv_label_create(btnMinutesDown, nullptr);
  lv_label_set_text(txtMinDown, "-");

  btnEnable = lv_btn_create(lv_scr_act(), nullptr);
  btnEnable->user_data = this;
  lv_obj_set_event_cb(btnEnable, btnEventHandler);
  lv_obj_align(btnEnable, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 3, -10);
  lv_obj_set_height(btnEnable, 40);
  txtEnable = lv_label_create(btnEnable, nullptr);
  setEnableButtonState();

  btnRecur = lv_btn_create(lv_scr_act(), nullptr);
  btnRecur->user_data = this;
  lv_obj_set_event_cb(btnRecur, btnEventHandler);
  lv_obj_align(btnRecur, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -3, -10);
  lv_obj_set_height(btnRecur, 40);
  txtRecur = lv_label_create(btnRecur, nullptr);
  setRecurButtonState();

  btnInfo = lv_btn_create(lv_scr_act(), nullptr);
  btnInfo->user_data = this;
  lv_obj_set_event_cb(btnInfo, btnEventHandler);
  lv_obj_align(btnInfo, lv_scr_act(), LV_ALIGN_CENTER, 30, -80);
  lv_obj_set_height(btnInfo, 40);
  lv_obj_set_width(btnInfo, 30);
  txtInfo = lv_label_create(btnInfo, nullptr);
  lv_label_set_text(txtInfo, "i");
}

Alarm::~Alarm() {
  lv_obj_clean(lv_scr_act());
}

void Alarm::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  using Pinetime::Controllers::AlarmController;
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnEnable) {
      if (alarmController.State() == AlarmController::AlarmState::Alerting) {
        alarmController.StopAlerting();
      } else if (alarmController.State() == AlarmController::AlarmState::Set) {
        alarmController.DisableAlarm();
      } else {
        alarmController.SetAlarm(alarmHours, alarmMinutes);
      }
      setEnableButtonState();
      return;
    }
    if (obj == btnInfo) {
      showInfo();
      return;
    }
    if (obj == btnMessage) {
      lv_obj_del(txtMessage);
      lv_obj_del(btnMessage);
      txtMessage = nullptr;
      btnMessage = nullptr;
      return;
    }
    // If any other button was pressed, disable the alarm
    // this is to make it clear that the alarm won't be set until it is turned back on
    // this avoids calling the AlarmController to change the alarm time every time the user hits minute-up or minute-down;
    // can just do it once when the alarm is re-enabled
    if (alarmController.State() == AlarmController::AlarmState::Set) {
      alarmController.DisableAlarm();
      setEnableButtonState();
    }
    if (obj == btnMinutesUp) {
      if (alarmMinutes >= 59) {
        alarmMinutes = 0;
      } else {
        alarmMinutes++;
      }
      lv_label_set_text_fmt(time, "%02d:%02d", alarmHours, alarmMinutes);
      return;
    }
    if (obj == btnMinutesDown) {
      if (alarmMinutes == 0) {
        alarmMinutes = 59;
      } else {
        alarmMinutes--;
      }
      lv_label_set_text_fmt(time, "%02d:%02d", alarmHours, alarmMinutes);
      return;
    }
    if (obj == btnHoursUp) {
      if (alarmHours >= 23) {
        alarmHours = 0;
      } else {
        alarmHours++;
      }
      lv_label_set_text_fmt(time, "%02d:%02d", alarmHours, alarmMinutes);
      return;
    }
    if (obj == btnHoursDown) {
      if (alarmHours == 0) {
        alarmHours = 23;
      } else {
        alarmHours--;
      }
      lv_label_set_text_fmt(time, "%02d:%02d", alarmHours, alarmMinutes);
      return;
    }
    if (obj == btnRecur) {
      alarmController.ToggleRecurrence();
      setRecurButtonState();
    }
  }
}

void Alarm::SetAlerting() {
  setEnableButtonState();
}

void Alarm::setEnableButtonState() {
  switch (alarmController.State()) {
    case AlarmController::AlarmState::Set:
      lv_label_set_text(txtEnable, "ON");
      lv_obj_set_style_local_bg_color(btnEnable, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
      break;
    case AlarmController::AlarmState::Not_Set:
      lv_label_set_text(txtEnable, "OFF");
      lv_obj_set_style_local_bg_color(btnEnable, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
      break;
    case AlarmController::AlarmState::Alerting:
      lv_label_set_text(txtEnable, Symbols::stop);
      lv_obj_set_style_local_bg_color(btnEnable, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  }
}

void Alarm::showInfo() {
  btnMessage = lv_btn_create(lv_scr_act(), nullptr);
  btnMessage->user_data = this;
  lv_obj_set_event_cb(btnMessage, btnEventHandler);
  lv_obj_set_height(btnMessage, 200);
  lv_obj_set_width(btnMessage, 150);
  lv_obj_align(btnMessage, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  txtMessage = lv_label_create(btnMessage, nullptr);
  lv_obj_set_style_local_bg_color(btnMessage, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_NAVY);

  if (alarmController.State() == AlarmController::AlarmState::Set) {
    auto timeToAlarm = alarmController.SecondsToAlarm();

    auto daysToAlarm = timeToAlarm / 86400;
    auto hrsToAlarm = (timeToAlarm % 86400) / 3600;
    auto minToAlarm = (timeToAlarm % 3600) / 60;
    auto secToAlarm = timeToAlarm % 60;

    lv_label_set_text_fmt(
      txtMessage, "Time to\nalarm:\n%2d Days\n%2d Hours\n%2d Minutes\n%2d Seconds", daysToAlarm, hrsToAlarm, minToAlarm, secToAlarm);
  } else {
    lv_label_set_text(txtMessage, "Alarm\nis not\nset.");
  }
}

void Alarm::setRecurButtonState() {
  using Pinetime::Controllers::AlarmController;
  switch (alarmController.Recurrence()) {
    case AlarmController::RecurType::None:
      lv_label_set_text(txtRecur, "ONCE");
      break;
    case AlarmController::RecurType::Daily:
      lv_label_set_text(txtRecur, "DAILY");
      break;
    case AlarmController::RecurType::Weekdays:
      lv_label_set_text(txtRecur, "WKDAYS");
  }
}