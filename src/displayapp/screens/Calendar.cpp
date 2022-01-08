#include "displayapp/screens/Calendar.h"
#include "displayapp/DisplayApp.h"
#include "components/datetime/DateTimeController.h"

using namespace Pinetime::Applications::Screens;

Calendar::Calendar(DisplayApp* app, Controllers::DateTime& dateTimeController) : Screen(app), dateTimeController {dateTimeController} {

    // Create calendar object
    lv_obj_t  * calendar = lv_calendar_create(lv_scr_act(), NULL);
    // Set size
    lv_obj_set_size(calendar, 240, 240);
    // Set alignment
    lv_obj_align(calendar, NULL, LV_ALIGN_CENTER, 0, 0);

    // Set style of today's date
    lv_obj_set_style_local_text_color(calendar, LV_CALENDAR_PART_DATE, LV_STATE_FOCUSED, LV_COLOR_RED);
    // Set style of days of week
    lv_obj_set_style_local_text_color(calendar, LV_CALENDAR_PART_DAY_NAMES, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x50, 0x50, 0x50));

    // Get today's date
    lv_calendar_date_t today;
    today.year = static_cast<int>(dateTimeController.Year());
    today.month = static_cast<int>(dateTimeController.Month());
    today.day = static_cast<int>(dateTimeController.Day());

    // Set today's date
    lv_calendar_set_today_date(calendar, &today);
    lv_calendar_set_showed_date(calendar, &today);

}

Calendar::~Calendar() {
    lv_obj_clean(lv_scr_act());
}
