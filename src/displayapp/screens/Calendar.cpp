/*  Copyright (C) 2024 thnikk, Boteium, JustScott

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "displayapp/screens/Calendar.h"
#include "components/datetime/DateTimeController.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

Calendar::Calendar(Controllers::DateTime& dateTimeController) : dateTimeController {dateTimeController} {

  // Create calendar object
  calendar = lv_calendar_create(lv_scr_act(), NULL);
  // Set size
  lv_obj_set_size(calendar, LV_HOR_RES, LV_VER_RES);
  // Set alignment
  lv_obj_align(calendar, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
  // Disable clicks
  lv_obj_set_click(calendar, false);

  // Set style of today's date
  lv_obj_set_style_local_text_color(calendar, LV_CALENDAR_PART_DATE, LV_STATE_FOCUSED, Colors::deepOrange);

  // Set style of inactive month's days
  lv_obj_set_style_local_text_color(calendar, LV_CALENDAR_PART_DATE, LV_STATE_DISABLED, Colors::gray);

  // Get today's date
  current.year = static_cast<int>(dateTimeController.Year());
  current.month = static_cast<int>(dateTimeController.Month());
  current.day = static_cast<int>(dateTimeController.Day());

  // Set today's date
  lv_calendar_set_today_date(calendar, &current);
  lv_calendar_set_showed_date(calendar, &current);
}

bool Calendar::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeLeft: {
      if (current.month == 12) {
        current.month = 1;
        current.year++;
      } else {
        current.month++;
      }

      lv_calendar_set_showed_date(calendar, &current);
      return true;
    }
    case TouchEvents::SwipeRight: {
      if (current.month == 1) {
        current.month = 12;
        current.year--;
      } else {
        current.month--;
      }

      lv_calendar_set_showed_date(calendar, &current);
      return true;
    }
    /*
    case TouchEvents::SwipeUp: {
        current.year++;
        lv_calendar_set_showed_date(calendar, &current);
        return true;
    }
    case TouchEvents::SwipeDown: {
        current.year--;
        lv_calendar_set_showed_date(calendar, &current);
        return true;
    }
    */
    default: {
      return false;
    }
  }
}

Calendar::~Calendar() {
  lv_obj_clean(lv_scr_act());
}
