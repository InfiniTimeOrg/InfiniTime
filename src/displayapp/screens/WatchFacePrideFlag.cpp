#include "displayapp/screens/WatchFacePrideFlag.h"
#include <lvgl/lvgl.h>
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void EventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<WatchFacePrideFlag*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }

  Pinetime::Controllers::Settings::PrideFlag GetNext(Pinetime::Controllers::Settings::PrideFlag prideFlag) {
    const auto prideFlagAsInt = static_cast<uint8_t>(prideFlag);
    Pinetime::Controllers::Settings::PrideFlag nextFlag;
    if (prideFlagAsInt < 3) {
      nextFlag = static_cast<Pinetime::Controllers::Settings::PrideFlag>(prideFlagAsInt + 1);
    } else {
      nextFlag = static_cast<Pinetime::Controllers::Settings::PrideFlag>(0);
    }
    return nextFlag;
  }

  Pinetime::Controllers::Settings::PrideFlag GetPrevious(Pinetime::Controllers::Settings::PrideFlag prideFlag) {
    const auto prideFlagAsInt = static_cast<uint8_t>(prideFlag);
    Pinetime::Controllers::Settings::PrideFlag prevFlag;
    if (prideFlagAsInt > 0) {
      prevFlag = static_cast<Pinetime::Controllers::Settings::PrideFlag>(prideFlagAsInt - 1);
    } else {
      prevFlag = static_cast<Pinetime::Controllers::Settings::PrideFlag>(3);
    }
    return prevFlag;
  }

  template <std::size_t N>
  class PrideFlagData {
  public:
    constexpr PrideFlagData(const std::array<lv_color_t, N>& sectionColours,
                            lv_color_t defaultTopLabelColour,
                            lv_color_t labelTimeColour,
                            lv_color_t defaultBottomLabelColour)
      : sectionColours {sectionColours},
        defaultTopLabelColour {defaultTopLabelColour},
        labelTimeColour {labelTimeColour},
        defaultBottomLabelColour {defaultBottomLabelColour} {
      // Space between adjacent text values calculated according to the following equation
      spacing = static_cast<uint8_t>(1.5f * static_cast<float>(N) + 40.5f);
    }

    std::array<lv_color_t, N> sectionColours;
    lv_color_t defaultTopLabelColour;
    lv_color_t labelTimeColour;
    lv_color_t defaultBottomLabelColour;
    uint8_t spacing;
  };

  constexpr lv_color_t lightBlue = LV_COLOR_MAKE(0x00, 0xbf, 0xf3);
  constexpr lv_color_t lightPink = LV_COLOR_MAKE(0xf4, 0x9a, 0xc1);
  constexpr lv_color_t hotPink = LV_COLOR_MAKE(0xd6, 0x02, 0x70);
  constexpr lv_color_t grayPurple = LV_COLOR_MAKE(0x9b, 0x4f, 0x96);
  constexpr lv_color_t darkBlue = LV_COLOR_MAKE(0x00, 0x38, 0xa8);
  constexpr lv_color_t orange = LV_COLOR_MAKE(0xef, 0x76, 0x27);
  constexpr lv_color_t lightOrange = LV_COLOR_MAKE(0xff, 0x9b, 0x55);
  constexpr lv_color_t lightPurple = LV_COLOR_MAKE(0xd4, 0x61, 0xa6);
  constexpr lv_color_t darkPurple = LV_COLOR_MAKE(0xb5, 0x56, 0x90);
  constexpr lv_color_t magenta = LV_COLOR_MAKE(0xa5, 0x00, 0x62);
  constexpr lv_color_t darkGreen = LV_COLOR_MAKE(0x07, 0x8d, 0x70);
  constexpr lv_color_t cyan = LV_COLOR_MAKE(0x26, 0xce, 0xaa);
  constexpr lv_color_t lightGreen = LV_COLOR_MAKE(0x98, 0xe8, 0xc1);
  constexpr lv_color_t indigo = LV_COLOR_MAKE(0x50, 0x49, 0xcc);
  constexpr lv_color_t steelBlue = LV_COLOR_MAKE(0x3d, 0x1a, 0x78);
  constexpr std::array<lv_color_t, 7> gayColours {darkGreen, cyan, lightGreen, LV_COLOR_WHITE, lightBlue, indigo, steelBlue};
  constexpr std::array<lv_color_t, 5> transColours {lightBlue, lightPink, LV_COLOR_WHITE, lightPink, lightBlue};
  constexpr std::array<lv_color_t, 5> biColours {hotPink, hotPink, grayPurple, darkBlue, darkBlue};
  constexpr std::array<lv_color_t, 7> lesbianColours {LV_COLOR_RED, orange, lightOrange, LV_COLOR_WHITE, lightPurple, darkPurple, magenta};
  constexpr PrideFlagData gayFlagData(gayColours, LV_COLOR_BLACK, LV_COLOR_BLACK, LV_COLOR_WHITE);
  constexpr PrideFlagData transFlagData(transColours, LV_COLOR_WHITE, LV_COLOR_BLACK, LV_COLOR_WHITE);
  constexpr PrideFlagData biFlagData(biColours, LV_COLOR_BLACK, LV_COLOR_WHITE, LV_COLOR_BLACK);
  constexpr PrideFlagData lesbianFlagData(lesbianColours, LV_COLOR_WHITE, LV_COLOR_BLACK, LV_COLOR_WHITE);
}

WatchFacePrideFlag::WatchFacePrideFlag(Controllers::DateTime& dateTimeController,
                                       const Controllers::Battery& batteryController,
                                       const Controllers::Ble& bleController,
                                       Controllers::NotificationManager& notificationManager,
                                       Controllers::Settings& settingsController,
                                       Controllers::MotionController& motionController)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    motionController {motionController} {

  bluetoothStatus = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(bluetoothStatus, "");
  lv_obj_align(bluetoothStatus, nullptr, LV_ALIGN_IN_TOP_RIGHT, -16, 0);

  batteryValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(batteryValue, true);
  lv_label_set_align(batteryValue, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(batteryValue, true);

  notificationText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(notificationText, nullptr, LV_ALIGN_IN_LEFT_MID, 0, -110);
  lv_obj_set_style_local_text_color(notificationText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  btnClose = lv_btn_create(lv_scr_act(), nullptr);
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_local_bg_opa(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblClose = lv_label_create(btnClose, nullptr);
  lv_label_set_text_static(lblClose, "X");
  lv_obj_set_event_cb(btnClose, EventHandler);
  lv_obj_set_hidden(btnClose, true);

  btnNextFlag = lv_btn_create(lv_scr_act(), nullptr);
  btnNextFlag->user_data = this;
  lv_obj_set_size(btnNextFlag, 60, 60);
  lv_obj_align(btnNextFlag, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 80);
  lv_obj_set_style_local_bg_opa(btnNextFlag, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblNextBG = lv_label_create(btnNextFlag, nullptr);
  lv_label_set_text_static(lblNextBG, ">");
  lv_obj_set_event_cb(btnNextFlag, EventHandler);
  lv_obj_set_hidden(btnNextFlag, true);

  btnPrevFlag = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevFlag->user_data = this;
  lv_obj_set_size(btnPrevFlag, 60, 60);
  lv_obj_align(btnPrevFlag, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 80);
  lv_obj_set_style_local_bg_opa(btnPrevFlag, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblPrevFlag = lv_label_create(btnPrevFlag, nullptr);
  lv_label_set_text_static(lblPrevFlag, "<");
  lv_obj_set_event_cb(btnPrevFlag, EventHandler);
  lv_obj_set_hidden(btnPrevFlag, true);

  labelDate = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(labelDate, true);
  lv_label_set_align(labelDate, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(labelDate, true);

  labelTime = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(labelTime, true);
  lv_obj_align(labelTime, lv_scr_act(), LV_ALIGN_CENTER, 0, -1);
  lv_label_set_align(labelTime, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_style_local_text_font(labelTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_auto_realign(labelTime, true);

  labelDay = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(labelDay, true);
  lv_label_set_align(labelDay, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(labelDay, true);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(stepValue, true);
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_CENTER, 0, 96);
  lv_label_set_align(stepValue, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(stepValue, true);

  UpdateScreen(settingsController.GetPrideFlag());

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFacePrideFlag::~WatchFacePrideFlag() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

bool WatchFacePrideFlag::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && lv_obj_get_hidden(btnClose)) {
    lv_obj_set_hidden(btnPrevFlag, false);
    lv_obj_set_hidden(btnNextFlag, false);
    lv_obj_set_hidden(btnClose, false);
    savedTick = xTaskGetTickCount();
    return true;
  }
  if ((event == Pinetime::Applications::TouchEvents::DoubleTap) && !lv_obj_get_hidden(btnClose)) {
    return true;
  }
  return false;
}

void WatchFacePrideFlag::CloseMenu() {
  settingsController.SaveSettings();
  lv_obj_set_hidden(btnClose, true);
  lv_obj_set_hidden(btnNextFlag, true);
  lv_obj_set_hidden(btnPrevFlag, true);
}

void WatchFacePrideFlag::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  bleState = bleController.IsConnected();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated() || themeChanged) {
    lv_label_set_text_fmt(batteryValue, "%d%%", batteryPercentRemaining.Get());
    if (batteryController.IsPowerPresent()) {
      lv_label_ins_text(batteryValue, LV_LABEL_POS_LAST, " Charging");
    }
  }
  if (bleState.IsUpdated()) {
    if (bleState.Get()) {
      lv_label_set_text_static(bluetoothStatus, Symbols::bluetooth);
    } else {
      lv_label_set_text_static(bluetoothStatus, "");
    }
  }

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    if (notificationState.Get()) {
      lv_label_set_text_static(notificationText, "You have\nmail!");
    } else {
      lv_label_set_text_static(notificationText, "");
    }
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated() || themeChanged) {
    uint8_t hour = dateTimeController.Hours();
    const uint8_t minute = dateTimeController.Minutes();
    const uint8_t second = dateTimeController.Seconds();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      if (hour == 0) {
        ampmChar = "AM";
        hour = 12;
      } else if (hour == 12) {
        ampmChar = "PM";
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar = "PM";
      }
    }

    lv_label_set_text_fmt(labelTime, "%02d:%02d:%02d", hour, minute, second);

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated() || ampmChar.IsUpdated() || themeChanged) {
      const uint16_t year = dateTimeController.Year();
      const Controllers::DateTime::Months month = dateTimeController.Month();
      const uint8_t day = dateTimeController.Day();
      const Controllers::DateTime::Days dayOfWeek = dateTimeController.DayOfWeek();
      lv_label_set_text_fmt(labelDate, "%02d-%02d-%04d", day, static_cast<uint8_t>(month), year);
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        lv_label_set_text_fmt(labelDay, "%s %s", dateTimeController.DayOfWeekToStringLow(dayOfWeek), ampmChar);
      } else {
        lv_label_set_text_fmt(labelDay, "%s", dateTimeController.DayOfWeekToStringLow(dayOfWeek));
      }
    }
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated() || themeChanged) {
    lv_label_set_text_fmt(stepValue, "%lu steps", stepCount.Get());
  }
  if (themeChanged) {
    themeChanged = false;
  }
}

void WatchFacePrideFlag::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    auto valueFlag = settingsController.GetPrideFlag();
    bool flagChanged = false;
    if (object == btnClose) {
      CloseMenu();
    }
    if (object == btnNextFlag) {
      valueFlag = GetNext(valueFlag);
      flagChanged = true;
    }
    if (object == btnPrevFlag) {
      valueFlag = GetPrevious(valueFlag);
      flagChanged = true;
    }
    settingsController.SetPrideFlag(valueFlag);
    if (flagChanged) {
      UpdateScreen(valueFlag);
    }
  }
}

bool WatchFacePrideFlag::OnButtonPushed() {
  if (!lv_obj_get_hidden(btnClose)) {
    CloseMenu();
    return true;
  }
  return false;
}

void WatchFacePrideFlag::UpdateScreen(const Pinetime::Controllers::Settings::PrideFlag prideFlag) {
  auto UseFlagData = [this]<size_t N>(PrideFlagData<N> flagData) {
    backgroundSections.reserve(N);
    for (size_t i = 0; i < N; i++) {
      backgroundSections.push_back(lv_obj_create(lv_scr_act(), nullptr));
      lv_obj_set_style_local_bg_color(backgroundSections[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, flagData.sectionColours[i]);
      lv_obj_set_size(backgroundSections[i], LV_HOR_RES, (LV_VER_RES / N) + 1);
      lv_obj_set_pos(backgroundSections[i], 0, i * LV_VER_RES / N);
      lv_obj_set_style_local_radius(backgroundSections[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
      lv_obj_move_background(backgroundSections[i]);
    }
    lv_obj_set_style_local_text_color(labelTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, flagData.labelTimeColour);
    lv_obj_set_style_local_text_color(batteryValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, flagData.defaultTopLabelColour);
    lv_obj_set_style_local_text_color(labelDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, flagData.defaultTopLabelColour);
    lv_obj_set_style_local_text_color(labelDay, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, flagData.defaultBottomLabelColour);
    lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, flagData.defaultBottomLabelColour);
    lv_obj_align(batteryValue, lv_scr_act(), LV_ALIGN_CENTER, 0, -2 * flagData.spacing);
    lv_obj_align(labelDate, lv_scr_act(), LV_ALIGN_CENTER, 0, -1 * flagData.spacing);
    lv_obj_align(labelDay, lv_scr_act(), LV_ALIGN_CENTER, 0, flagData.spacing);
    lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_CENTER, 0, 2 * flagData.spacing);
  };
  themeChanged = true;
  for (lv_obj_t* backgroundSection : backgroundSections) {
    lv_obj_del(backgroundSection);
  }
  backgroundSections.clear();
  switch (prideFlag) {
    case Pinetime::Controllers::Settings::PrideFlag::Gay:
      UseFlagData(gayFlagData);
      break;
    case Pinetime::Controllers::Settings::PrideFlag::Trans:
      UseFlagData(transFlagData);
      break;
    case Pinetime::Controllers::Settings::PrideFlag::Bi:
      UseFlagData(biFlagData);
      break;
    case Pinetime::Controllers::Settings::PrideFlag::Lesbian:
      UseFlagData(lesbianFlagData);
      break;
  }
}
