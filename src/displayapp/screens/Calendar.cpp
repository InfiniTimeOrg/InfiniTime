#include "displayapp/screens/Calendar.h"
#include "displayapp/DisplayApp.h"
#include "components/datetime/DateTimeController.h"
#include "displayapp/screens/BatteryIcon.h"

using namespace Pinetime::Applications::Screens;

Calendar::Calendar(DisplayApp* app, Pinetime::Controllers::Battery& batteryController, Controllers::DateTime& dateTimeController) : Screen(app), batteryController {batteryController}, dateTimeController {dateTimeController} {

    // Status bar clock and battery
    label_time = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text(label_time, dateTimeController.FormattedTime().c_str());
    lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);
    batteryIcon = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryController.PercentRemaining()));
    lv_obj_align(batteryIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

    // Create calendar object
    calendar = lv_calendar_create(lv_scr_act(), NULL);
    // Set size
    lv_obj_set_size(calendar, 230, 200);
    // Set alignment
    lv_obj_align(calendar, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    // Disable clicks
    lv_obj_set_click(calendar, false);

    // Set background of today's date
    lv_obj_set_style_local_bg_opa(calendar, LV_CALENDAR_PART_DATE, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(calendar, LV_CALENDAR_PART_DATE, LV_STATE_FOCUSED, LV_COLOR_WHITE);
    lv_obj_set_style_local_radius(calendar, LV_CALENDAR_PART_DATE, LV_STATE_FOCUSED, 3);

    // Set style of today's date
    lv_obj_set_style_local_text_color(calendar, LV_CALENDAR_PART_DATE, LV_STATE_FOCUSED, LV_COLOR_BLACK);

    // Set style of inactive month's days
    lv_obj_set_style_local_text_color(calendar, LV_CALENDAR_PART_DATE, LV_STATE_DISABLED, lv_color_hex(0x505050));

    // Get today's date
    today.year = static_cast<int>(dateTimeController.Year());
    today.month = static_cast<int>(dateTimeController.Month());
    today.day = static_cast<int>(dateTimeController.Day());

    // Set today's date
    lv_calendar_set_today_date(calendar, &today);
    lv_calendar_set_showed_date(calendar, &today);

    // Use today's date as a reference for which month to show if moved
    current = today;

}

bool Calendar::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
    switch (event) {
        case TouchEvents::SwipeLeft: {
            if (current.month == 12) {
                current.month = 1;
                current.year++;
            }
            else{
                current.month++;
            }
            lv_calendar_set_showed_date(calendar, &current);
            return true;
        }
        case TouchEvents::SwipeRight: {
            if (current.month == 1) {
                current.month = 12;
                current.year--;
            }
            else{
                current.month--;
            }
            lv_calendar_set_showed_date(calendar, &current);
            return true;
        }
        default: {
            return false;
        }
    }
}

Calendar::~Calendar() {
    lv_obj_clean(lv_scr_act());
}
