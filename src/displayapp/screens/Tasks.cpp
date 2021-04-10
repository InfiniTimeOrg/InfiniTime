#include <FreeRTOS.h>
#include <task.h>
#include "Tasks.h"
#include <lvgl/lvgl.h>
#include "../DisplayApp.h"
#include <string>
#include <algorithm>

using namespace Pinetime::Applications::Screens;

static void lv_update_task(struct _lv_task_t *task) {  
  auto user_data = static_cast<Tasks *>(task->user_data);
  user_data->UpdateScreen();
}

Tasks::Tasks(
  Pinetime::Applications::DisplayApp *app) : 
  Screen(app)
{

  table = lv_table_create(lv_scr_act(), NULL);
  lv_table_set_col_cnt(table, 3);
  lv_table_set_row_cnt(table, 8);
  //lv_obj_align(table, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_size(table, 240, 240);
  lv_obj_set_pos(table, 0, 0);

  /*lv_table_set_cell_type(table, 0, 0, 1);
  lv_table_set_cell_type(table, 0, 1, 1);
  lv_table_set_cell_type(table, 0, 2, 1);
  lv_table_set_cell_type(table, 0, 3, 1);*/

  lv_table_set_cell_value(table, 0, 0, "#");
  lv_table_set_col_width(table, 0, 50);
  lv_table_set_cell_value(table, 0, 1, "Task");
  lv_table_set_col_width(table, 1, 80);
  lv_table_set_cell_value(table, 0, 2, "Free");
  lv_table_set_col_width(table, 2, 80);

  lv_obj_t * backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  UpdateScreen();
  taskUpdate = lv_task_create(lv_update_task, 100000, LV_TASK_PRIO_LOW, this);

}

Tasks::~Tasks() {
  lv_task_del(taskUpdate);
  lv_obj_clean(lv_scr_act());
}

bool sortById(const TaskStatus_t &lhs, const TaskStatus_t &rhs) { return lhs.xTaskNumber < rhs.xTaskNumber; }

void Tasks::UpdateScreen() { 
  auto nb = uxTaskGetSystemState(tasksStatus, 7, nullptr);
  std::sort(tasksStatus, tasksStatus + nb, sortById);
  for (uint8_t i = 0; i < nb; i++) {
    
    lv_table_set_cell_value(table, i + 1, 0, std::to_string(tasksStatus[i].xTaskNumber).c_str());
    lv_table_set_cell_value(table, i + 1, 1, tasksStatus[i].pcTaskName);    
    if (tasksStatus[i].usStackHighWaterMark < 20) {
      std::string str1 = std::to_string(tasksStatus[i].usStackHighWaterMark) + " low";
      lv_table_set_cell_value(table, i + 1, 2, str1.c_str());
    } else {
      lv_table_set_cell_value(table, i + 1, 2, std::to_string(tasksStatus[i].usStackHighWaterMark).c_str());
    }

  }
}

bool Tasks::Refresh() { 
  return running;
}