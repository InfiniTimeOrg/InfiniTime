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

  TimeTrackingMode savedCurrMode = timeTrackerController.CurrMode();
  if ( savedCurrMode != TimeTrackingMode::Iddle ) {
    auto btnId = static_cast<int>(savedCurrMode);
    currMode = savedCurrMode;
    lv_btnmatrix_set_btn_ctrl(btnm1, btnId, LV_BTNMATRIX_CTRL_CHECK_STATE);
  }

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
  if (currMode != TimeTrackingMode::Iddle) {
    timeTrackerController.ModeChanged(currMode);
  }
  btnMessage = lv_btn_create(lv_scr_act(), nullptr);
  btnMessage->user_data = this;
  lv_obj_set_event_cb(btnMessage, btnHandler);
  lv_obj_set_height(btnMessage, 240);
  lv_obj_set_width(btnMessage, 240);
  lv_obj_align(btnMessage, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  txtMessage = lv_label_create(btnMessage, nullptr);
  lv_obj_set_style_local_bg_color(btnMessage, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_NAVY);

  TimePretty_t modeWork = convertTicksToTimeSegments(timeTrackerController.TimeInMode(TimeTrackingMode::Work));
  TimePretty_t modePlay = convertTicksToTimeSegments(timeTrackerController.TimeInMode(TimeTrackingMode::Play));
  TimePretty_t modeChores = convertTicksToTimeSegments(timeTrackerController.TimeInMode(TimeTrackingMode::Chores));
  TimePretty_t modeSocial = convertTicksToTimeSegments(timeTrackerController.TimeInMode(TimeTrackingMode::Social));
  TimePretty_t modeHealth = convertTicksToTimeSegments(timeTrackerController.TimeInMode(TimeTrackingMode::Health));
  TimePretty_t modeLearn = convertTicksToTimeSegments(timeTrackerController.TimeInMode(TimeTrackingMode::Learn));

  lv_label_set_text_fmt(txtMessage,
                        "Work  : %02d:%02d:%02d\nPlay  : %02d:%02d:%02d\nChores: %02d:%02d:%02d\nSocial: %02d:%02d:%02d\nHealth: "
                        "%02d:%02d:%02d\nLearn : %02d:%02d:%02d\n",
                        modeWork.hours,
                        modeWork.mins,
                        modeWork.secs,
                        modePlay.hours,
                        modePlay.mins,
                        modePlay.secs,
                        modeChores.hours,
                        modeChores.mins,
                        modeChores.secs,
                        modeSocial.hours,
                        modeSocial.mins,
                        modeSocial.secs,
                        modeHealth.hours,
                        modeHealth.mins,
                        modeHealth.secs,
                        modeLearn.hours,
                        modeLearn.mins,
                        modeLearn.secs);

  btnReset = lv_btn_create(btnMessage, nullptr);
  btnReset->user_data = this;
  lv_obj_set_event_cb(btnReset, btnHandler);
  lv_obj_set_height(btnReset, 50);
  lv_obj_set_width(btnReset, 100);
  //lv_obj_align(btnReset, btnMessage, LV_ALIGN_IN_TOP_MID, 0, 0);
  txtReset = lv_label_create(btnReset, nullptr);
  lv_label_set_text_static(txtReset, "Reset");
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
    auto newMode = static_cast<TimeTrackingMode>(btnId);
    currMode = newMode;
    timeTrackerController.ModeChanged(newMode);
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

    if (obj == btnReset) {
      printf("Reset\n");
      timeTrackerController.Reset();
      CloseInfo();
    }

    return;
  }

  // TickType_t currTime = xTaskGetTickCount();
  // startTime = currTime;
  // currMode = static_cast<TimeTrackingMode>(btnId);
}