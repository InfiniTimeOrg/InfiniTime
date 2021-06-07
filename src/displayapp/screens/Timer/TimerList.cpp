#include "TimerList.h"

using namespace Pinetime::Applications::Screens;

TimerList::TimerList(uint8_t screenID,
                     uint8_t numScreens,
                     DisplayApp* app,
                     Controllers::TimerController& timerController)
    : List(app, screenID, numScreens), timerController{timerController} {
  
  
  char names[3][6];
  for (int i = 0; i < 3; i++) {
    sprintf(names[i], "%02d:%02d", timerController.getPreset(i).minutes, timerController.getPreset(i).seconds);
  }
  std::array<List::Item, MAXLISTITEMS> timerItems {{
      {"", "Set", nullptr},
      {"", names[0], nullptr},
      {"", names[1], nullptr},
      {"", names[2], nullptr},
  }};
  
  for (int i = 0; i < MAXLISTITEMS; i++) {
    createButtonNr(i, timerItems[i]);
  }
  lv_btn_set_checkable(buttons[0], true);
  
}

TimerList::~TimerList() {
  lv_obj_clean(lv_scr_act());
}

bool TimerList::Refresh() {
  
  return running;
}

void TimerList::OnButtonEvent(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_RELEASED) {
    switch (currentMode) {
      case Normal:
        if (object == buttons[0]) {
          
          currentMode = Set;
          return;
        }
        for (int i = 1; i < MAXLISTITEMS; i++) {
          if (object == buttons[i]) {
            //first button is "Set"
            timerController.selectPreset(i -1);
            app->StartApp(Apps::Timer, DisplayApp::FullRefreshDirections::LeftAnim);
            return;
          }
        }
        break;
      case Set:
        if (object == buttons[0]) {
          Item newItem = {"", "Set", nullptr};
          createButtonNr(0, newItem);
          currentMode = Normal;
          return;
        }
        for (int i = 1; i < MAXLISTITEMS; i++) {
          if (object == buttons[i]) {
            //first button is "Set"
            timerController.setPreset(i - 1);
            char newTxt[6];
            sprintf(newTxt, "%02d:%02d", timerController.getPreset(i).minutes, timerController.getPreset(i).seconds);
            Item newItem = {"", newTxt, nullptr};
            createButtonNr(i, newItem);
            app->StartApp(Apps::Timer, DisplayApp::FullRefreshDirections::LeftAnim);
            return;
          }
        }
    }
    
  }
}

bool TimerList::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  
  if (event == TouchEvents::SwipeLeft) {
    app->StartApp(Apps::Timer, DisplayApp::FullRefreshDirections::LeftAnim);
  }
  return false;
}
