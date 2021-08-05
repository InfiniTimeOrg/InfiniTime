#include "WatchFaceBinary.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "BatteryIcon.h"
#include "BleIcon.h"
#include "NotificationIcon.h"
#include "Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;

WatchFaceBinary::WatchFaceBinary(DisplayApp* app,
                                   Controllers::DateTime& dateTimeController,
                                   Controllers::Battery& batteryController,
                                   Controllers::Ble& bleController,
                                   Controllers::NotificationManager& notificatioManager,
                                   Controllers::Settings& settingsController,
                                   Controllers::HeartRateController& heartRateController,
                                   Controllers::MotionController& motionController)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController}
{
  settingsController.SetClockFace(3);

  minutes_old = 0;

  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -5, 2);

  batteryPlug = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryPlug, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDD0000));
  lv_label_set_text(batteryPlug, Symbols::plug);
  lv_obj_align(batteryPlug, label_volt, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0000FF));
  lv_label_set_text(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  notificationIcon = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00DD00));
  lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 10, 0);



  label_year = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_year, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 5, (LED_SIZE1));
  lv_obj_set_style_local_text_color(label_year, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  
  label_day = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_day, label_year, LV_ALIGN_OUT_BOTTOM_MID, 0, (LED_SPACE_V2));  
  lv_obj_set_style_local_text_color(label_day, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));  
  

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 5, -2);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_label_set_text(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text(stepValue, "0");
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -2);

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  
  
  
  //////////////////
  // Binary Watch //
  //////////////////

  // Generate Circles //
  //////////////////////
  
  // for minutes //
  // from left to right, start with MSB
  
  minLED5 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(minLED5, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_OFF);
  lv_obj_set_style_local_radius(minLED5, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(minLED5, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING1);
  lv_obj_set_style_local_line_color(minLED5,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);  
  lv_obj_set_size(minLED5, LED_SIZE1, LED_SIZE1);
  lv_obj_align(minLED5, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, LED_SPACE_H1/2, (5-LED_SIZE1/2));
  
  minLED4 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(minLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_OFF);
  lv_obj_set_style_local_radius(minLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(minLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING1);
  lv_obj_set_style_local_line_color(minLED4,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);  
  lv_obj_set_size(minLED4, LED_SIZE1, LED_SIZE1);
  lv_obj_align(minLED4, minLED5, LV_ALIGN_OUT_RIGHT_MID, LED_SPACE_H1, 0);

  minLED3 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(minLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_OFF);
  lv_obj_set_style_local_radius(minLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(minLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING1);
  lv_obj_set_style_local_line_color(minLED3,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);  
  lv_obj_set_size(minLED3, LED_SIZE1, LED_SIZE1);
  lv_obj_align(minLED3, minLED4, LV_ALIGN_OUT_RIGHT_MID, LED_SPACE_H1, 0);    
  
  minLED2 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(minLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_OFF);
  lv_obj_set_style_local_radius(minLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(minLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING1);
  lv_obj_set_style_local_line_color(minLED2,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);
  lv_obj_set_size(minLED2, LED_SIZE1, LED_SIZE1);
  lv_obj_align(minLED2, minLED3, LV_ALIGN_OUT_RIGHT_MID, LED_SPACE_H1, 0);    
  
  minLED1 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(minLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_OFF);
  lv_obj_set_style_local_radius(minLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(minLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING1);
  lv_obj_set_style_local_line_color(minLED1,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);    
  lv_obj_set_size(minLED1, LED_SIZE1, LED_SIZE1);
  lv_obj_align(minLED1, minLED2, LV_ALIGN_OUT_RIGHT_MID, LED_SPACE_H1, 0);    
  
  minLED0 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(minLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_OFF);
  lv_obj_set_style_local_radius(minLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(minLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING1);
  lv_obj_set_style_local_line_color(minLED0,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);    
  lv_obj_set_size(minLED0, LED_SIZE1, LED_SIZE1);
  lv_obj_align(minLED0, minLED1, LV_ALIGN_OUT_RIGHT_MID, LED_SPACE_H1, 0);    
  
  // for hours //
  // from left to right, start with MSB
  
  hourLED4 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(hourLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_OFF);
  lv_obj_set_style_local_radius(hourLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(hourLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING1);
  lv_obj_set_style_local_line_color(hourLED4,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);    
  lv_obj_set_size(hourLED4, LED_SIZE1, LED_SIZE1);
  lv_obj_align(hourLED4, minLED4, LV_ALIGN_OUT_BOTTOM_MID, 0, (-(2*LED_SIZE1+LED_SPACE_V1)));  

  hourLED3 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(hourLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_OFF);
  lv_obj_set_style_local_radius(hourLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(hourLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING1);
  lv_obj_set_style_local_line_color(hourLED3,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);      
  lv_obj_set_size(hourLED3, LED_SIZE1, LED_SIZE1);
  lv_obj_align(hourLED3, hourLED4, LV_ALIGN_OUT_RIGHT_MID, LED_SPACE_H1, 0);    
  
  hourLED2 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(hourLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_OFF);
  lv_obj_set_style_local_radius(hourLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(hourLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING1);
  lv_obj_set_style_local_line_color(hourLED2,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);      
  lv_obj_set_size(hourLED2, LED_SIZE1, LED_SIZE1);
  lv_obj_align(hourLED2, hourLED3, LV_ALIGN_OUT_RIGHT_MID, LED_SPACE_H1, 0);    
  
  hourLED1 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(hourLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_OFF);
  lv_obj_set_style_local_radius(hourLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(hourLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING1);
  lv_obj_set_style_local_line_color(hourLED1,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);      
  lv_obj_set_size(hourLED1, LED_SIZE1, LED_SIZE1);
  lv_obj_align(hourLED1, hourLED2, LV_ALIGN_OUT_RIGHT_MID, LED_SPACE_H1, 0);    
  
  hourLED0 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(hourLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_OFF);
  lv_obj_set_style_local_radius(hourLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(hourLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING1);
  lv_obj_set_style_local_line_color(hourLED0,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);      
  lv_obj_set_size(hourLED0, LED_SIZE1, LED_SIZE1);
  lv_obj_align(hourLED0, hourLED1, LV_ALIGN_OUT_RIGHT_MID, LED_SPACE_H1, 0);  


  // for days //
  // oriented at right border, so alignment must be from right to left, start with LSB
  
  dayLED0 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(dayLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_OFF);
  lv_obj_set_style_local_radius(dayLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(dayLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING2);
  lv_obj_set_style_local_line_color(dayLED0,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING); 
  lv_obj_set_size(dayLED0, LED_SIZE2, LED_SIZE2);
  lv_obj_align(dayLED0, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -5, (LED_SIZE1));
  
  dayLED1 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(dayLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_OFF);
  lv_obj_set_style_local_radius(dayLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(dayLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING2);
  lv_obj_set_style_local_line_color(dayLED1,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);   
  lv_obj_set_size(dayLED1, LED_SIZE2, LED_SIZE2);
  lv_obj_align(dayLED1, dayLED0, LV_ALIGN_OUT_LEFT_MID, -LED_SPACE_H2, 0);  
  
  dayLED2 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(dayLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_OFF);
  lv_obj_set_style_local_radius(dayLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(dayLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING2);
  lv_obj_set_style_local_line_color(dayLED2,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);
  lv_obj_set_size(dayLED2, LED_SIZE2, LED_SIZE2);
  lv_obj_align(dayLED2, dayLED1, LV_ALIGN_OUT_LEFT_MID, -LED_SPACE_H2, 0);    
  
  dayLED3 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(dayLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_OFF);
  lv_obj_set_style_local_radius(dayLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(dayLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING2);
  lv_obj_set_style_local_line_color(dayLED3,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);  
  lv_obj_set_size(dayLED3, LED_SIZE2, LED_SIZE2);
  lv_obj_align(dayLED3, dayLED2, LV_ALIGN_OUT_LEFT_MID, -LED_SPACE_H2, 0);    
  
  dayLED4 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(dayLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_OFF);
  lv_obj_set_style_local_radius(dayLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(dayLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING2);
  lv_obj_set_style_local_line_color(dayLED4,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);  
  lv_obj_set_size(dayLED4, LED_SIZE2, LED_SIZE2);
  lv_obj_align(dayLED4, dayLED3, LV_ALIGN_OUT_LEFT_MID, -LED_SPACE_H2, 0);

  
  // for months //
  // aligns with days, normal order is possible -> from left to right, start with MSB

  monLED3 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(monLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MON_OFF);
  lv_obj_set_style_local_radius(monLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(monLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING2);
  lv_obj_set_style_local_line_color(monLED3,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);      
  lv_obj_set_size(monLED3, LED_SIZE2, LED_SIZE2);
  lv_obj_align(monLED3, dayLED3, LV_ALIGN_OUT_BOTTOM_MID, 0, (LED_SPACE_V2));  
  
  monLED2 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(monLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MON_OFF);
  lv_obj_set_style_local_radius(monLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(monLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING2);
  lv_obj_set_style_local_line_color(monLED2,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);      
  lv_obj_set_size(monLED2, LED_SIZE2, LED_SIZE2);
  lv_obj_align(monLED2, monLED3, LV_ALIGN_OUT_RIGHT_MID, LED_SPACE_H2, 0);    
  
  monLED1 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(monLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MON_OFF);
  lv_obj_set_style_local_radius(monLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(monLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING2);
  lv_obj_set_style_local_line_color(monLED1,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);      
  lv_obj_set_size(monLED1, LED_SIZE2, LED_SIZE2);
  lv_obj_align(monLED1, monLED2, LV_ALIGN_OUT_RIGHT_MID, LED_SPACE_H2, 0);    
  
  monLED0 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(monLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MON_OFF);
  lv_obj_set_style_local_radius(monLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(monLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_RING2);
  lv_obj_set_style_local_line_color(monLED0,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_RING);      
  lv_obj_set_size(monLED0, LED_SIZE2, LED_SIZE2);
  lv_obj_align(monLED0, monLED1, LV_ALIGN_OUT_RIGHT_MID, LED_SPACE_H2, 0);  
}

WatchFaceBinary::~WatchFaceBinary()
{
  lv_obj_clean(lv_scr_act());
}

bool WatchFaceBinary::Refresh()
{
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) 
  {
    auto batteryPercent = batteryPercentRemaining.Get();
    lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryPercent));
    auto isCharging = batteryController.IsCharging() || batteryController.IsPowerPresent();
    lv_label_set_text(batteryPlug, BatteryIcon::GetPlugIcon(isCharging));
  }

  bleState = bleController.IsConnected();
  if (bleState.IsUpdated())
  {
    if (bleState.Get() == true)
    {
      lv_label_set_text(bleIcon, BleIcon::GetIcon(true));
    }
    else
    {
      lv_label_set_text(bleIcon, BleIcon::GetIcon(false));
    }
  }
  lv_obj_align(batteryIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -5, 2);
  lv_obj_align(label_volt, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  lv_obj_align(batteryPlug, label_volt, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  lv_obj_align(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);


  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated())
  {
    if (notificationState.Get() == true)
      lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(true));
    else
      lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(false));
  }

  currentDateTime = dateTimeController.CurrentDateTime();

  if (currentDateTime.IsUpdated()) 
  {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime);
    auto time = date::make_time(newDateTime - dp);
    auto yearMonthDay = date::year_month_day(dp);

    auto year = (int) yearMonthDay.year();
    auto month = static_cast<Pinetime::Controllers::DateTime::Months>((unsigned) yearMonthDay.month());
    auto day = (unsigned) yearMonthDay.day();
    auto dayOfWeek = static_cast<Pinetime::Controllers::DateTime::Days>(date::weekday(yearMonthDay).iso_encoding());

    int hour = time.hours().count();
    auto minute = time.minutes().count();
    auto second = time.seconds().count();


    // Binary Watch //
    if (minutes_old != minutes)
    {
      uint8_t binMinTmp = static_cast<int>(minute);
      uint8_t binHourTmp = static_cast<int>(hour);
      
      // minutes //
      if (binMinTmp >= 32)
      {
        binMinTmp -= 32;
        lv_obj_set_style_local_bg_color(minLED5, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(minLED5, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_OFF);
      }
      
      if (binMinTmp >= 16)
      {
        binMinTmp -= 16;
        lv_obj_set_style_local_bg_color(minLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(minLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_OFF);
      }  
      
      if (binMinTmp >= 8)
      {
        binMinTmp -= 8;
        lv_obj_set_style_local_bg_color(minLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(minLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_OFF);
      }
      
      if (binMinTmp >= 4)
      {
        binMinTmp -= 4;
        lv_obj_set_style_local_bg_color(minLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_ON);
      }    
      else      
      {
        lv_obj_set_style_local_bg_color(minLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_OFF);
      }
      
      if (binMinTmp >= 2)
      {
        binMinTmp -= 2;
        lv_obj_set_style_local_bg_color(minLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(minLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_OFF);
      }
      
      if (binMinTmp == 1)
      {
        lv_obj_set_style_local_bg_color(minLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(minLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MIN_OFF);
      }      
      
      // hours //
      if (binHourTmp >= 16)
      {
        binHourTmp -= 16;
        lv_obj_set_style_local_bg_color(hourLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(hourLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_OFF);
      }  
      
      if (binHourTmp >= 8)
      {
        binHourTmp -= 8;
        lv_obj_set_style_local_bg_color(hourLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(hourLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_OFF);
      }
      
      if (binHourTmp >= 4)
      {
        binHourTmp -= 4;
        lv_obj_set_style_local_bg_color(hourLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_ON);
      }    
      else      
      {
        lv_obj_set_style_local_bg_color(hourLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_OFF);
      }
      
      if (binHourTmp >= 2)
      {
        binHourTmp -= 2;
        lv_obj_set_style_local_bg_color(hourLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(hourLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_OFF);
      }
      
      if (binHourTmp == 1)
      {
        lv_obj_set_style_local_bg_color(hourLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(hourLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_HOUR_OFF);
      }      
      
      minutes_old = minutes;
    }
    // End Binary Watch //


    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) 
    {
      char dateStr[6];
      sprintf(dateStr, "%d", year);    
      lv_label_set_text(label_year, dateStr);

      char dayStr[11];
      sprintf(dayStr, "%s", dateTimeController.DayOfWeekToString());    
      lv_label_set_text(label_day, dayStr);

      // Binary Date //
      uint8_t binDayTmp = static_cast<int>(day);
      uint8_t binMonTmp = static_cast<int>(month);
      
      // Days //  
      if (binDayTmp >= 16)
      {
        binDayTmp -= 16;
        lv_obj_set_style_local_bg_color(dayLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(dayLED4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_OFF);
      }  
      
      if (binDayTmp >= 8)
      {
        binDayTmp -= 8;
        lv_obj_set_style_local_bg_color(dayLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(dayLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_OFF);
      }
      
      if (binDayTmp >= 4)
      {
        binDayTmp -= 4;
        lv_obj_set_style_local_bg_color(dayLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_ON);
      }    
      else      
      {
        lv_obj_set_style_local_bg_color(dayLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_OFF);
      }
      
      if (binDayTmp >= 2)
      {
        binDayTmp -= 2;
        lv_obj_set_style_local_bg_color(dayLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(dayLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_OFF);
      }
      
      if (binDayTmp == 1)
      {
        lv_obj_set_style_local_bg_color(dayLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(dayLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_DAY_OFF);
      }      
      
      // Months //
      if (binMonTmp >= 8)
      {
        binMonTmp -= 8;
        lv_obj_set_style_local_bg_color(monLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MON_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(monLED3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MON_OFF);
      }
      
      if (binMonTmp >= 4)
      {
        binMonTmp -= 4;
        lv_obj_set_style_local_bg_color(monLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MON_ON);
      }    
      else      
      {
        lv_obj_set_style_local_bg_color(monLED2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MON_OFF);
      }
      
      if (binMonTmp >= 2)
      {
        binMonTmp -= 2;
        lv_obj_set_style_local_bg_color(monLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MON_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(monLED1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MON_OFF);
      }
      
      if (binMonTmp == 1)
      {
        lv_obj_set_style_local_bg_color(monLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MON_ON);
      }
      else
      {
        lv_obj_set_style_local_bg_color(monLED0, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LED_COL_MON_OFF);
      }      
      // Ende Binary Date //
    }

    currentYear = year;
    currentMonth = month;
    currentDayOfWeek = dayOfWeek;
    currentDay = day;
  }


  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) 
  {
    if (heartbeatRunning.Get()) 
    {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    }
    else
    {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1B1B1B));
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 5, -2);
    lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
  }

  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) 
  {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -2);
    lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }

  return running;
}
