#pragma once

#include <cstdint>
#include <chrono>
#include "displayapp/screens/Screen.h"
#include <lvgl/src/lv_core/lv_style.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include "components/datetime/DateTimeController.h"
#include <unordered_map>
#include <string>


namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Classes : public Screen {
      public:
        Classes(Pinetime::Controllers::DateTime& dateTimeController);
        
        //int findNextClass(std::unordered_map<std::string, int>& num_list,
        //                  std::unordered_map<int, std::string>& start_list,
        //                  Pinetime::Controllers::DateTime& dateTimeController);
        int findNextClass(Pinetime::Controllers::DateTime& dateTimeController) {


        //std::string timeUntilNextClass(int nextClassNumber,
        //                               Pinetime::Controllers::DateTime& dateTimeController);
        std::string Classes::formatTime(const std::string& timeStr) {


       // Classes();
        ~Classes() override;

        void Refresh() override;

      private:

        Pinetime::Controllers::DateTime& dateTimeController;
      
        std::unordered_map<std::string, int> num_list;
        std::unordered_map<int, std::string> start_list;
        std::unordered_map<int, std::string> end_list;
        std::unordered_map<int, std::string> name_list;
        std::unordered_map<int, std::string> loc_list;
     //   std::unordered_map<std::string, std::string> list;
       // std::unordered_map<std::string, std::string> list;

        lv_obj_t* label;
        lv_obj_t* next_class_start;
        lv_obj_t* next_class_end;
        lv_obj_t* next_class_name;
        lv_obj_t* next_class_loc;
        lv_obj_t* next_class_dif;

        lv_task_t* taskRefresh;

      };
    }
  }
}
