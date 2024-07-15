/*********************************************************/
/*
 * I modified the watchface Infineat :
 * - added alarm info on the screen
 * - modified the colors
 * - modified step count icon
 * Except colors, modifications are at line 254 and 500
 */
/*********************************************************/



#include "displayapp/screens/WatchFaceMeow.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/BleIcon.h"
//#include "displayapp/screens/AlarmIcon.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/alarm/AlarmController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<WatchFaceMeow*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }

  enum class colors {
    orange,
    blue,
    green,
    rainbow,
    vivid,
    pink,
    nordGreen,
  };

  constexpr int nColors = 7; // must match number of colors in InfineatColorsColors

  constexpr int nLines = WatchFaceMeow::nLines;

  constexpr std::array<lv_color_t, nLines> orangeColors = {LV_COLOR_MAKE(0xfd, 0x87, 0x2b),
                                                           LV_COLOR_MAKE(0xdb, 0x33, 0x16),
                                                           LV_COLOR_MAKE(0x6f, 0x10, 0x00),
                                                           LV_COLOR_MAKE(0xfd, 0x7a, 0x0a),
                                                           LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                           LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                           LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                           LV_COLOR_MAKE(0xe8, 0x51, 0x02),
                                                           LV_COLOR_MAKE(0xea, 0x1c, 0x00)};
  constexpr std::array<lv_color_t, nLines> blueColors = {LV_COLOR_MAKE(0xe7, 0xf8, 0xff),
                                                         LV_COLOR_MAKE(0x22, 0x32, 0xd0),
                                                         LV_COLOR_MAKE(0x18, 0x2a, 0x8b),
                                                         LV_COLOR_MAKE(0xe7, 0xf8, 0xff),
                                                         LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                         LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                         LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                         LV_COLOR_MAKE(0x59, 0x91, 0xff),
                                                         LV_COLOR_MAKE(0x16, 0x36, 0xff)};
  constexpr std::array<lv_color_t, nLines> greenColors = {LV_COLOR_MAKE(0xb8, 0xff, 0x9b),
                                                          LV_COLOR_MAKE(0x08, 0x86, 0x08),
                                                          LV_COLOR_MAKE(0x00, 0x4a, 0x00),
                                                          LV_COLOR_MAKE(0xb8, 0xff, 0x9b),
                                                          LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                          LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                          LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                          LV_COLOR_MAKE(0x62, 0xd5, 0x15),
                                                          LV_COLOR_MAKE(0x00, 0x74, 0x00)};
  //added comments to say which color is which triangle
  constexpr std::array<lv_color_t, nLines> rainbowColors = {LV_COLOR_MAKE(0x2d, 0xa4, 0x00),  //green, small triangle on top
                                                            LV_COLOR_MAKE(0xac, 0x09, 0xc4),  //purple, smalltriangle in the bottom half part on the clock display side
                                                            LV_COLOR_MAKE(0xfe, 0x03, 0x03),  //red, the two small triangles above and below the battery
                                                            LV_COLOR_MAKE(0x0d, 0x57, 0xff),  //blue, the small triangle at the bottom
                                                            LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                            LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                            LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                            LV_COLOR_MAKE(0xe0, 0xb9, 0x00),  //Yellow, large triangle on top left
                                                            LV_COLOR_MAKE(0xe8, 0x51, 0x02)}; //orange, large triangle on bottom left

// Add new colors, still rainbow but more pastel like
  constexpr std::array<lv_color_t, nLines> rainbowVividColors ={LV_COLOR_MAKE(0xa5, 0xeb, 0x64),
                                                         	LV_COLOR_MAKE(0xfc, 0x42, 0xb5),
                                                         	LV_COLOR_MAKE(0xe7, 0xc1, 0xff),
                                                         	LV_COLOR_MAKE(0x11, 0xdf, 0xfa),
                                                         	LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                         	LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                         	LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                         	LV_COLOR_MAKE(0xff, 0xec, 0x5d),
                                                         	LV_COLOR_MAKE(0xff, 0x93, 0xaf)};

  constexpr std::array<lv_color_t, nLines> pinkColors =     {LV_COLOR_MAKE(0xff, 0xe5, 0xec),
                                                             LV_COLOR_MAKE(0xff, 0xb3, 0xc6),
                                                             LV_COLOR_MAKE(0xfb, 0x6f, 0x92),
                                                             LV_COLOR_MAKE(0xff, 0xe5, 0xec),
                                                             LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                             LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                             LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                             LV_COLOR_MAKE(0xff, 0xc2, 0xd1),
                                                             LV_COLOR_MAKE(0xff, 0x8f, 0xab)};
  constexpr std::array<lv_color_t, nLines> nordGreenColors = {LV_COLOR_MAKE(0xd5, 0xf0, 0xe9),
                                                              LV_COLOR_MAKE(0x23, 0x83, 0x73),
                                                              LV_COLOR_MAKE(0x1d, 0x41, 0x3f),
                                                              LV_COLOR_MAKE(0xd5, 0xf0, 0xe9),
                                                              LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                              LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                              LV_COLOR_MAKE(0xff, 0xff, 0xff),
                                                              LV_COLOR_MAKE(0x2f, 0xb8, 0xa2),
                                                              LV_COLOR_MAKE(0x11, 0x70, 0x5a)};

  //define colors for texts and symbols
  // gray is used for text symbols and time. I changed it to pink, because I can.
  //static constexpr lv_color_t grayColor = LV_COLOR_MAKE(0x99, 0x99, 0x99);
  static constexpr lv_color_t pinkColor = LV_COLOR_MAKE(0xfc, 0x42, 0xb5);
 
  constexpr const std::array<lv_color_t, nLines>* returnColor(colors color) {
    if (color == colors::orange) {
      return &orangeColors;
    }
    if (color == colors::blue) {
      return &blueColors;
    }
    if (color == colors::green) {
      return &greenColors;
    }
    if (color == colors::rainbow) {
      return &rainbowColors;
    }
    if (color == colors::vivid) {
      return &rainbowVividColors;
    }
    if (color == colors::pink) {
      return &pinkColors;
    }
    return &nordGreenColors;
  }
}

WatchFaceMeow::WatchFaceMeow(Controllers::DateTime& dateTimeController,
                                     const Controllers::Battery& batteryController,
                                     const Controllers::Ble& bleController,
                                     Controllers::AlarmController& alarmController,
                                     Controllers::NotificationManager& notificationManager,
                                     Controllers::Settings& settingsController,
                                     Controllers::MotionController& motionController,
                                     Controllers::FS& filesystem)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    alarmController {alarmController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    motionController {motionController} {
  lfs_file f = {};
  if (filesystem.FileOpen(&f, "/fonts/teko.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_teko = lv_font_load("F:/fonts/teko.bin");
  }

  if (filesystem.FileOpen(&f, "/fonts/bebas.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_bebas = lv_font_load("F:/fonts/bebas.bin");
  }

  // Side Cover
  //paires de points pour chaque ligne
  // les lignes sont pas les contours des triangles, elles sont des grosses bandes superposées

  static constexpr lv_point_t linePoints[nLines][2] = {{{30, 25}, {68, -8}}, //1 small triangle on top
                                                       {{26, 167}, {43, 216}}, //2 purple, smalltriangle in the bottom half part on the clock display side
                                                       {{27, 40}, {27, 196}},// 3 small triangles up above and below battery
                                                       {{12, 182}, {65, 249}}, //4 most bottom right triangle
                                                       {{17, 97}, {17, 147}}, // 5 left part of battery zone, overlapped after by the large triangles
                                                       {{16, 81}, {42, 127}}, //6 upper part of battery zone
                                                       {{16, 163}, {42, 118}}, //7 lower part of battery zone
                                                       {{-20, 124}, {25, -11}}, //8 large upper triangle
                                                       {{-29, 89}, {27, 254}}}; //9 large lower triangle
  //largeur des bandes
  static constexpr lv_style_int_t lineWidths[nLines] = {18, 15, 14, 22, 20, 18, 18, 52, 48};

  const std::array<lv_color_t, nLines>* colors = returnColor(static_cast<enum colors>(settingsController.GetInfineatColorIndex()));
  for (int i = 0; i < nLines; i++) {
    lines[i] = lv_line_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_line_width(lines[i], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lineWidths[i]);
    lv_color_t color = (*colors)[i];
    lv_obj_set_style_local_line_color(lines[i], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, color);
    lv_line_set_points(lines[i], linePoints[i], 2);
  }

  //Battery indicator
  logoCat = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(logoCat, "F:/images/cat_small.bin");
  lv_obj_set_pos(logoCat, 12, 108);
  //adjust position for cat
  lineBattery = lv_line_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_line_width(lineBattery, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 30);
  lv_obj_set_style_local_line_color(lineBattery, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, (*colors)[4]);
  lv_obj_set_style_local_line_opa(lineBattery, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 190);
  lineBatteryPoints[0] = {27, 107};//27 = image x offset + image width / 2
  lineBatteryPoints[1] = {27, 108};// the line covering the image is initialized as 1 px high
  lv_line_set_points(lineBattery, lineBatteryPoints, 2);
  lv_obj_move_foreground(lineBattery);

  notificationIcon = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(notificationIcon, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, (*colors)[7]);
  lv_obj_set_style_local_radius(notificationIcon, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_size(notificationIcon, 13, 13);
  lv_obj_set_hidden(notificationIcon, true);

  if (!settingsController.GetInfineatShowSideCover()) {
    ToggleBatteryIndicatorColor(false);
    for (auto& line : lines) {
      lv_obj_set_hidden(line, true);
    }
  }

  timeContainer = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_opa(timeContainer, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_size(timeContainer, 185, 185);
  lv_obj_align(timeContainer, lv_scr_act(), LV_ALIGN_CENTER, 0, -10);

  labelHour = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelHour, "01");
  lv_obj_set_style_local_text_font(labelHour, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_bebas);
  lv_obj_set_style_local_text_color(labelHour, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, pinkColor);
  lv_obj_align(labelHour, timeContainer, LV_ALIGN_IN_TOP_MID, 0, 0);

  labelMinutes = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelMinutes, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_bebas);
  lv_obj_set_style_local_text_color(labelMinutes, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, pinkColor);
  lv_label_set_text_static(labelMinutes, "00");
  lv_obj_align(labelMinutes, timeContainer, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  labelTimeAmPm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelTimeAmPm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_teko);
  lv_obj_set_style_local_text_color(labelTimeAmPm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, pinkColor);

  lv_label_set_text_static(labelTimeAmPm, "");
  lv_obj_align(labelTimeAmPm, timeContainer, LV_ALIGN_OUT_RIGHT_TOP, 0, 15);

  dateContainer = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_opa(dateContainer, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_size(dateContainer, 60, 30);
  lv_obj_align(dateContainer, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 5);

  labelDate = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, pinkColor);
  lv_obj_set_style_local_text_font(labelDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_teko);
  lv_obj_align(labelDate, dateContainer, LV_ALIGN_IN_TOP_MID, 0, 0);
  lv_label_set_text_static(labelDate, "Mon 01");

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, pinkColor);
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, dateContainer, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);


  // Based on existing code, I understand that items on the screen (date, bluteooth status..)
  // are declared here with default states, and later below the state (date, ...) is assigned
  // So I do the same to add the alarm status : I put a symbol that has a default value
  // and a text that has a default value
  
  // text
  labelAlarm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelAlarm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, pinkColor);
  lv_obj_set_style_local_text_font(labelAlarm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_teko);
  //lv_obj_align(labelAlarm, dateContainer, LV_ALIGN_OUT_BOTTOM_MID, -10, 0);
  lv_obj_align(labelAlarm, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -3, 0);
  lv_label_set_text_static(labelAlarm, "00:00");

  labelTimeAmPmAlarm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelTimeAmPmAlarm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_teko);
  lv_label_set_text_static(labelTimeAmPmAlarm, "");
  lv_obj_set_style_local_text_color(labelTimeAmPmAlarm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, pinkColor);
  lv_obj_align(labelTimeAmPmAlarm, labelAlarm, LV_ALIGN_OUT_TOP_RIGHT, 0, 0);

  // symbol
  alarmIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(alarmIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, pinkColor);
  lv_label_set_text_static(alarmIcon, Symbols::zzz);
  lv_obj_align(alarmIcon, labelAlarm, LV_ALIGN_OUT_LEFT_MID, -3, 0);

  // don't show the icons jsut set if we don't show alarm status
  if (!settingsController.GetInfineatShowAlarmStatus()) {
    lv_obj_set_hidden(labelAlarm, true);
    lv_obj_set_hidden(alarmIcon, true);
    lv_obj_set_hidden(labelTimeAmPmAlarm, true);
  }

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, pinkColor);
  lv_obj_set_style_local_text_font(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_teko);
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 10, 0);
  lv_label_set_text_static(stepValue, "0");

  pawIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(pawIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, pinkColor);
  lv_label_set_text_static(pawIcon, Symbols::paw);
  lv_obj_align(pawIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);


  // Setting buttons
  btnClose = lv_btn_create(lv_scr_act(), nullptr);
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_local_bg_opa(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_t* lblClose = lv_label_create(btnClose, nullptr);
  lv_label_set_text_static(lblClose, "X");
  lv_obj_set_event_cb(btnClose, event_handler);
  lv_obj_set_hidden(btnClose, true);

  btnNextColor = lv_btn_create(lv_scr_act(), nullptr);
  btnNextColor->user_data = this;
  lv_obj_set_size(btnNextColor, 60, 60);
  lv_obj_align(btnNextColor, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 0);
  lv_obj_set_style_local_bg_opa(btnNextColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_t* lblNextColor = lv_label_create(btnNextColor, nullptr);
  lv_label_set_text_static(lblNextColor, ">");
  lv_obj_set_event_cb(btnNextColor, event_handler);
  lv_obj_set_hidden(btnNextColor, true);

  btnPrevColor = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevColor->user_data = this;
  lv_obj_set_size(btnPrevColor, 60, 60);
  lv_obj_align(btnPrevColor, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 0);
  lv_obj_set_style_local_bg_opa(btnPrevColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_t* lblPrevColor = lv_label_create(btnPrevColor, nullptr);
  lv_label_set_text_static(lblPrevColor, "<");
  lv_obj_set_event_cb(btnPrevColor, event_handler);
  lv_obj_set_hidden(btnPrevColor, true);

  btnToggleCover = lv_btn_create(lv_scr_act(), nullptr);
  btnToggleCover->user_data = this;
  lv_obj_set_size(btnToggleCover, 60, 60);
  lv_obj_align(btnToggleCover, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(btnToggleCover, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  const char* labelToggle = settingsController.GetInfineatShowSideCover() ? "ON" : "OFF";
  lblToggle = lv_label_create(btnToggleCover, nullptr);
  lv_label_set_text_static(lblToggle, labelToggle);
  lv_obj_set_event_cb(btnToggleCover, event_handler);
  lv_obj_set_hidden(btnToggleCover, true);

  btnToggleAlarm = lv_btn_create(lv_scr_act(), nullptr);
  btnToggleAlarm->user_data = this;
  lv_obj_set_size(btnToggleAlarm, 60, 60);
  lv_obj_align(btnToggleAlarm, lv_scr_act(), LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_local_bg_opa(btnToggleAlarm, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  const char* labelToggleAlarm = settingsController.GetInfineatShowAlarmStatus() ? Symbols::bell : Symbols::notbell;
  lblAlarm = lv_label_create(btnToggleAlarm, nullptr);
  lv_label_set_text_static(lblAlarm, labelToggleAlarm);
  lv_obj_set_event_cb(btnToggleAlarm, event_handler);
  lv_obj_set_hidden(btnToggleAlarm, true);

  // Button to access the settings
  btnSettings = lv_btn_create(lv_scr_act(), nullptr);
  btnSettings->user_data = this;
  lv_obj_set_size(btnSettings, 150, 150);
  lv_obj_align(btnSettings, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_radius(btnSettings, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 30);
  lv_obj_set_style_local_bg_opa(btnSettings, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_set_event_cb(btnSettings, event_handler);
  labelBtnSettings = lv_label_create(btnSettings, nullptr);
  lv_obj_set_style_local_text_font(labelBtnSettings, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(labelBtnSettings, Symbols::settings);
  lv_obj_set_hidden(btnSettings, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceMeow::~WatchFaceMeow() {
  lv_task_del(taskRefresh);

  if (font_bebas != nullptr) {
    lv_font_free(font_bebas);
  }
  if (font_teko != nullptr) {
    lv_font_free(font_teko);
  }

  lv_obj_clean(lv_scr_act());
}

bool WatchFaceMeow::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && lv_obj_get_hidden(btnSettings)) {
    lv_obj_set_hidden(btnSettings, false);
    savedTick = lv_tick_get();
    return true;
  }
  // Prevent screen from sleeping when double tapping with settings on
  if ((event == Pinetime::Applications::TouchEvents::DoubleTap) && !lv_obj_get_hidden(btnClose)) {
    return true;
  }
  return false;
}

void WatchFaceMeow::CloseMenu() {
  settingsController.SaveSettings();
  lv_obj_set_hidden(btnClose, true);
  lv_obj_set_hidden(btnNextColor, true);
  lv_obj_set_hidden(btnPrevColor, true);
  lv_obj_set_hidden(btnToggleCover, true);
  lv_obj_set_hidden(btnToggleAlarm, true);
}

bool WatchFaceMeow::OnButtonPushed() {
  if (!lv_obj_get_hidden(btnClose)) {
    CloseMenu();
    return true;
  }
  return false;
}

void WatchFaceMeow::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    bool showSideCover = settingsController.GetInfineatShowSideCover();
    int colorIndex = settingsController.GetInfineatColorIndex();
    bool showAlarmStatus = settingsController.GetInfineatShowAlarmStatus();

    if (object == btnSettings) {
      lv_obj_set_hidden(btnSettings, true);
      lv_obj_set_hidden(btnClose, false);
      lv_obj_set_hidden(btnNextColor, !showSideCover);
      lv_obj_set_hidden(btnPrevColor, !showSideCover);
      lv_obj_set_hidden(btnToggleCover, false);
      lv_obj_set_hidden(btnToggleAlarm, false);
    }
    if (object == btnClose) {
      CloseMenu();
    }
    if (object == btnToggleCover) {
      settingsController.SetInfineatShowSideCover(!showSideCover);
      ToggleBatteryIndicatorColor(!showSideCover);
      for (auto& line : lines) {
        lv_obj_set_hidden(line, showSideCover);
      }
      lv_obj_set_hidden(btnNextColor, showSideCover);
      lv_obj_set_hidden(btnPrevColor, showSideCover);
      const char* labelToggle = showSideCover ? "OFF" : "ON";
      lv_label_set_text_static(lblToggle, labelToggle);
    }

    if (object == btnToggleAlarm) {
      settingsController.SetInfineatShowAlarmStatus(!showAlarmStatus);
      bool newShowAlarmStatus = settingsController.GetInfineatShowAlarmStatus();
      lv_obj_set_hidden(labelAlarm, !newShowAlarmStatus);
      lv_obj_set_hidden(alarmIcon, !newShowAlarmStatus);
      lv_obj_set_hidden(labelTimeAmPmAlarm, !newShowAlarmStatus);
      const char* labelToggleAlarm = newShowAlarmStatus ? Symbols::bell : Symbols::notbell;
      lv_label_set_text_static(lblAlarm, labelToggleAlarm);
    }


    if (object == btnNextColor) {
      colorIndex = (colorIndex + 1) % nColors;
      settingsController.SetInfineatColorIndex(colorIndex);
    }
    if (object == btnPrevColor) {
      colorIndex -= 1;
      if (colorIndex < 0)
        colorIndex = nColors - 1;
      settingsController.SetInfineatColorIndex(colorIndex);
    }
    if (object == btnNextColor || object == btnPrevColor) {
      const std::array<lv_color_t, nLines>* colors = returnColor(static_cast<enum colors>(settingsController.GetInfineatColorIndex()));
      for (int i = 0; i < nLines; i++) {
        lv_color_t color = (*colors)[i];
        lv_obj_set_style_local_line_color(lines[i], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, color);
      }
      lv_obj_set_style_local_line_color(lineBattery, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, (*colors)[4]);
      lv_obj_set_style_local_bg_color(notificationIcon, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, (*colors)[7]);
    }
  }
}

void WatchFaceMeow::Refresh() {
  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_obj_set_hidden(notificationIcon, !notificationState.Get());
    lv_obj_align(notificationIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      char ampmChar[3] = "AM";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampmChar[0] = 'P';
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar[0] = 'P';
      }
      lv_label_set_text(labelTimeAmPm, ampmChar);
    }
    lv_label_set_text_fmt(labelHour, "%02d", hour);
    lv_label_set_text_fmt(labelMinutes, "%02d", minute);

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      lv_obj_align(labelTimeAmPm, timeContainer, LV_ALIGN_OUT_RIGHT_TOP, 0, 10);
      lv_obj_align(labelHour, timeContainer, LV_ALIGN_IN_TOP_MID, 0, 5);
      lv_obj_align(labelMinutes, timeContainer, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    }

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint8_t day = dateTimeController.Day();
      Controllers::DateTime::Days dayOfWeek = dateTimeController.DayOfWeek();
      lv_label_set_text_fmt(labelDate, "%s %02d", dateTimeController.DayOfWeekShortToStringLow(dayOfWeek), day);
      lv_obj_realign(labelDate);
    }
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  isCharging = batteryController.IsCharging();
  if (batteryController.IsCharging()) { // Charging battery animation
    chargingBatteryPercent += 1;
    if (chargingBatteryPercent > 100) {
      chargingBatteryPercent = batteryPercentRemaining.Get();
    }
    SetBatteryLevel(chargingBatteryPercent);
  } else if (isCharging.IsUpdated() || batteryPercentRemaining.IsUpdated()) {
    chargingBatteryPercent = batteryPercentRemaining.Get();
    SetBatteryLevel(chargingBatteryPercent);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated()) {
    //bleState.Get : in displayapp/widgets/StatusIcons.cpp:  bleState = bleController.IsConnected();
    //dynamic icons have their definitions in displayApp/screens/BleIcon.h / cpp
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
    lv_obj_align(bleIcon, dateContainer, LV_ALIGN_OUT_BOTTOM_MID, 0, 3);
  }

  // Add alarm state and time
  // AlarmState is an enum type in class AlarmController that is in namespace controllers
  if (settingsController.GetInfineatShowAlarmStatus()) {
    alarmState = alarmController.State()==Pinetime::Controllers::AlarmController::AlarmState::Set;
    // sets the icon as bird or bed
    const char* alarmSymbol = Symbols::zzz;
    if(alarmState) {
      alarmSymbol = Symbols::bird;
    }
    lv_label_set_text_static(alarmIcon, alarmSymbol);
    //displays the time of the alarm or nothing if the alarm is not set
    if (alarmState) {
      uint8_t alarmHours = alarmController.Hours();
      uint8_t alarmMinutes = alarmController.Minutes();
      //handles the am pm format.
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        char ampmChar[3] = "AM";
        if (alarmHours == 0) {
          alarmHours = 12;
          } else if (alarmHours == 12) {
            ampmChar[0]='P';
          } else if (alarmHours > 12) {
          alarmHours = alarmHours - 12;
          ampmChar[0]='P';
        }
        lv_label_set_text(labelTimeAmPmAlarm, ampmChar);
        lv_obj_set_style_local_text_font(labelTimeAmPmAlarm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_teko);
        lv_obj_align(labelTimeAmPmAlarm, labelAlarm, LV_ALIGN_OUT_TOP_RIGHT, 0, 0);
      }

      lv_label_set_text_fmt(labelAlarm, "%02d:%02d", alarmHours, alarmMinutes);

      lv_obj_align(alarmIcon, labelAlarm, LV_ALIGN_OUT_LEFT_MID, -3, 0);
      lv_obj_align(labelAlarm, dateContainer, LV_ALIGN_OUT_BOTTOM_MID, -10, 0);
      lv_obj_align(labelAlarm, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -3, 0);

    }
    else {
      lv_label_set_text_static(labelAlarm, Symbols::none);
      lv_obj_align(alarmIcon, dateContainer, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
      lv_obj_align(alarmIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -3, 0);
    }
  }


  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 10, 0);
    lv_obj_align(pawIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }

  if (!lv_obj_get_hidden(btnSettings)) {
    if ((savedTick > 0) && (lv_tick_get() - savedTick > 3000)) {
      lv_obj_set_hidden(btnSettings, true);
      savedTick = 0;
    }
  }
}

void WatchFaceMeow::SetBatteryLevel(uint8_t batteryPercent) {
  // starting point (y) + Pine64 logo height * (100 - batteryPercent) / 100^
  //the ligne grows, it covers the icon starting from the top
  lineBatteryPoints[1] = {27, static_cast<lv_coord_t>(107 + 31 * (100 - batteryPercent) / 100)};
  lv_line_set_points(lineBattery, lineBatteryPoints, 2);
}

void WatchFaceMeow::ToggleBatteryIndicatorColor(bool showSideCover) {
  if (!showSideCover) { // make indicator and notification icon color white
    lv_obj_set_style_local_image_recolor_opa(logoCat, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_image_recolor(logoCat, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_line_color(lineBattery, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_color(notificationIcon, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  } else {
    lv_obj_set_style_local_image_recolor_opa(logoCat, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    const std::array<lv_color_t, nLines>* colors = returnColor(static_cast<enum colors>(settingsController.GetInfineatColorIndex()));
    lv_obj_set_style_local_line_color(lineBattery, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, (*colors)[4]);
    lv_obj_set_style_local_bg_color(notificationIcon, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, (*colors)[7]);
  }
}

bool WatchFaceMeow::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/fonts/teko.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/bebas.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/images/cat_small.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  return true;
}
