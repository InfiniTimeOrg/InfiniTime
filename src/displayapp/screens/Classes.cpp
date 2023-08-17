//#include "displayapp/screens/Motion.h"
#include "displayapp/screens/Classes.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"
#include <vector>

using namespace Pinetime::Applications::Screens;

// Function to calculate time until the next class and return class number
std::string timeUntilNextClass(std::unordered_map<std::string, int>& num_list,
                                std::unordered_map<int, std::string>& start_list,
                                Pinetime::Controllers::DateTime& dateTimeController, int& nextClassNumber)

                               {
  //  auto currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());

    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();
    std::string dayOfWeek = dateTimeController.DayOfWeekShortToString();
    
    // Convert dayOfWeek to one-letter abbreviation
    std::string oneLetterDay;
    if      (dayOfWeek == "SUN") oneLetterDay = "U";
    else if (dayOfWeek == "MON") oneLetterDay = "M";
    else if (dayOfWeek == "TUE") oneLetterDay = "T";
    else if (dayOfWeek == "WED") oneLetterDay = "W";
    else if (dayOfWeek == "THU") oneLetterDay = "R";
    else if (dayOfWeek == "FRI") oneLetterDay = "F";
    else if (dayOfWeek == "SAT") oneLetterDay = "S";
    
    // Find the list of classes for the current day
    std::vector<int> classesForToday;
    for (const auto& entry : num_list) {
        if (entry.first.find(oneLetterDay) != std::string::npos) {
            classesForToday.push_back(entry.second);
        }
    }
    
  //  int currentDay = num_list[dayOfWeek];
    int currentMinutes = hour * 60 + minute;
    int closestClassTimeDiff = -1;
    
    for (int classNumber : classesForToday) {
        if (start_list.find(classNumber) != start_list.end()) {
            std::string classStartTime = start_list[classNumber];
            
            // Parse the class start time to extract hours and minutes
            size_t colonPos = classStartTime.find(':');
            int classStartHour = std::stoi(classStartTime.substr(0, colonPos));
            int classStartMinute = std::stoi(classStartTime.substr(colonPos + 1, 2));
            
            int classStartMinutes = classStartHour * 60 + classStartMinute;
            
            if (classStartMinutes > currentMinutes) {
                int minutesUntilClass = classStartMinutes - currentMinutes;
                if (closestClassTimeDiff == -1 || minutesUntilClass < closestClassTimeDiff) {
                    closestClassTimeDiff = minutesUntilClass;
                    nextClassNumber = classNumber;
                }
            }
        }
    }
    
    if (closestClassTimeDiff != -1) {
        int hoursUntilClass = closestClassTimeDiff / 60;
        int remainingMinutes = closestClassTimeDiff % 60;
        
        return std::to_string(hoursUntilClass) + ":" + std::to_string(remainingMinutes);
    }
  
    
    nextClassNumber = -1; // No class found
    return "";
}

//Classes::Classes(Controllers::DateTime& dateTimeController) {
//Classes::Classes() {
Classes::Classes(Pinetime::Controllers::DateTime& dateTimeController) {
   this->dateTimeController = dateTimeController;

  label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(label, "Next Class:", 0, 0, 0);
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, nullptr, LV_ALIGN_IN_TOP_LEFT, 5, 5);
  lv_label_set_recolor(label, true);

  next_class_name = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(next_class_name, "Loading Next Class", 0, 0, 0);
  lv_label_set_align(next_class_name, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(next_class_name, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
  lv_label_set_recolor(next_class_name, true);

  next_class_loc = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(next_class_loc, "Loading Class Loc", 0, 0, 0);
  lv_label_set_align(next_class_loc, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(next_class_loc, next_class_name, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
  lv_label_set_recolor(next_class_loc, true);

  next_class_start = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(next_class_start, "Loading Class Start", 0, 0, 0);
  lv_label_set_align(next_class_start, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(next_class_start, next_class_loc, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
  lv_label_set_recolor(next_class_start, true);

  next_class_end = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(next_class_end, "Loading Class End", 0, 0, 0);
  lv_label_set_align(next_class_end, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(next_class_end, next_class_start, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
  lv_label_set_recolor(next_class_end, true);

  next_class_dif = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(next_class_dif, "Loading Class End", 0, 0, 0);
  lv_label_set_align(next_class_dif, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(next_class_dif, next_class_end, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
  lv_label_set_recolor(next_class_dif, true);

  num_list["MWF"] = 1;
  num_list[  "W"] = 2;
  num_list[ "TR"] = 3;
  num_list[ "TR"] = 4;
  num_list["MWF"] = 5;
  num_list[ "TR"] = 6;
  
  name_list[1]    = "Comp Sci 2, 339";
  name_list[2]    = "Comp Lab 2, 329";
  name_list[3]    = "Film Appri, 108";
  name_list[4]    = "Health&Wel, 232";
  name_list[5]    = "Colg Algra, 212";
  name_list[6]    = "Colg Write, 202";
     
  loc_list[1]     = "Math CSci & Tech";
  loc_list[2]     = "Math CSci & Tech";
  loc_list[3]     = "Stanly Russ Hall";
  loc_list[4]     = "Doyne Heath Center";
  loc_list[5]     = "Math CSci & Tech";
  loc_list[6]     = "Win Thompson Hall";
  
  start_list[1]   = "10:00 AM";
  start_list[2]   = "12:00 PM";
  start_list[3]   =  "2:40 PM";
  start_list[4]   = "10:50 AM";
  start_list[5]   =  "9:00 AM";
  start_list[6]   =  "2:25 PM";
    
  end_list[1]     = "10:50 AM";
  end_list[2]     =  "1:50 PM";
  end_list[3]     =  "3:55 PM";
  end_list[4]     = "12:05 PM";
  end_list[5]     =  "9:50 AM";
  end_list[6]     = "10:40 AM";

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Classes::~Classes() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Classes::Refresh() {
  //lv_label_set_text_fmt(next_class, "#ff0fff hello chat!");

  //lv_label_set_text_fmt(label,            "%s", start_list[1]);

  int nextClassNumber;
  std::string result = timeUntilNextClass(num_list, start_list, dateTimeController, nextClassNumber);

  lv_label_set_text_fmt(next_class_name,  "%s",                          name_list[nextClassNumber].c_str());
  lv_label_set_text_fmt(next_class_loc,   "%s",                           loc_list[nextClassNumber].c_str());
  lv_label_set_text_fmt(next_class_start, "Start: %s",                  start_list[nextClassNumber].c_str());
  lv_label_set_text_fmt(next_class_end,   "End  : %s",                    end_list[nextClassNumber].c_str());
  lv_label_set_text_fmt(next_class_dif,   "In   : %s\nDay  : %s", result, "W");

}