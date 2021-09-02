#include "Screen.h"
using namespace Pinetime::Applications::Screens;

void Screen::RefreshTaskCallback(lv_task_t* task) {
  static_cast<Screen*>(task->user_data)->Refresh();
}
