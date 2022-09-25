#include "displayapp/screens/TrackTime.h"

#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  TimePretty_t convertTicksToTimeSegments(const TickType_t timeElapsed) {
    // Centiseconds
    const int timeElapsedCentis = timeElapsed * 100 / configTICK_RATE_HZ;

    const int hundredths = (timeElapsedCentis % 100);
    const int secs = (timeElapsedCentis / 100) % 60;
    const int mins = ((timeElapsedCentis / 100) / 60) % 60;
    const int hours = mins / 60;
    return TimePretty_t {hours, mins, secs};
  }

  void btnHandler(lv_obj_t* obj, lv_event_t event) {
    auto* tracktime = static_cast<TrackTime*>(obj->user_data);
    tracktime->handleModeUpdate(obj, event);
  }
}

TrackTime::TrackTime(DisplayApp* app, System::SystemTask& systemTask, Controllers::TimeTrackerController& timeTrackerController)
  : Screen(app), systemTask {systemTask}, timeTrackerController {timeTrackerController} {
  btnm1 = lv_btnmatrix_create(lv_scr_act(), nullptr);
  btnm1->user_data = this;
  lv_obj_align(btnm1, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_set_size(btnm1, 240, 190);
  lv_btnmatrix_set_map(btnm1, btnm_map);
  lv_btnmatrix_set_btn_ctrl_all(btnm1, LV_BTNMATRIX_CTRL_CHECKABLE);
  lv_obj_set_event_cb(btnm1, btnHandler);
  lv_btnmatrix_set_one_check(btnm1, true);

  btnSummary = lv_btn_create(lv_scr_act(), nullptr);
  btnSummary->user_data = this;
  lv_obj_set_event_cb(btnSummary, btnHandler);
  lv_obj_set_size(btnSummary, 220, 50);
  lv_obj_align(btnSummary, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  txtSummary = lv_label_create(btnSummary, nullptr);
  lv_label_set_text_static(txtSummary, "Summary");

}

TrackTime::~TrackTime() {
  lv_obj_clean(lv_scr_act());
}

/*
void TrackTime::Refresh() {
   if (currMode != TimeTrackingMode::Total) {
     TickType_t currTime = xTaskGetTickCount();
     TickType_t elapsedTime = currTime - startTime;
     totals[currMode] += elapsedTime;
     startTime = currTime;
     printf("%i %i\n", currMode, totals[currMode]);
     TimePretty_t total = convertTicksToTimeSegments(totals[currMode]);
     lv_label_set_text_fmt(title, "Total %02d:%02d:%02d", total.hours, total.mins, total.secs);
   }
}
*/

void TrackTime::ShowInfo() {
  btnMessage = lv_btn_create(lv_scr_act(), nullptr);
  btnMessage->user_data = this;
  lv_obj_set_event_cb(btnMessage, btnHandler);
  lv_obj_set_height(btnMessage, 240);
  lv_obj_set_width(btnMessage, 240);
  lv_obj_align(btnMessage, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  txtMessage = lv_label_create(btnMessage, nullptr);
  lv_obj_set_style_local_bg_color(btnMessage, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_NAVY);
  lv_label_set_text_static(txtMessage,
                           "Work  : 12:34:56\nPlay  : 12:34:56\nChores: 12:34:56\nSocial: 12:34:56\nHealth: 12:34:56\nLearn : 12:34:56\n");
}

void TrackTime::CloseInfo() {
  lv_obj_del(btnMessage);
  btnMessage = nullptr;
}

void TrackTime::handleModeUpdate(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    const char* txt = lv_btnmatrix_get_active_btn_text(btnm1);
    uint16_t btnId = lv_btnmatrix_get_active_btn(btnm1);
    printf("%s %i was pressed\n", txt, btnId);

    return;
  }

  if (event == LV_EVENT_CLICKED) {
    if (obj == btnMessage) {
      printf("Close Summary\n");
      CloseInfo();
    } else if (obj == btnSummary) {
      printf("Show Summary\n");
      timeTrackerController.Demo();
      ShowInfo();
    }

    return;
  }

  // TickType_t currTime = xTaskGetTickCount();
  // startTime = currTime;
  // currMode = static_cast<TimeTrackingMode>(btnId);
}