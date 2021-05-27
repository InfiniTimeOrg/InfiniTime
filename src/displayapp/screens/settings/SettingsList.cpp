#include "SettingsList.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;


SettingsList::SettingsList(uint8_t screenID,
                           uint8_t numScreens,
                           DisplayApp* app,
                           Controllers::Settings& settingsController,
                           std::array<Item, 4>& applications)
    : List(app, screenID, numScreens), settingsController{settingsController} {
  
  settingsController.SetSettingsMenu(screenID);
  
  
  for (int i = 0; i < MAXLISTITEMS; i++) {
    apps[i] = *static_cast<Pinetime::Applications::Apps*>(applications[i].data);
    //only create buttons for applications that exist
    if (apps[i] != Apps::None) {
      createButtonNr(i, applications[i]);
    }
  }
  
}

SettingsList::~SettingsList() {
  lv_obj_clean(lv_scr_act());
}

bool SettingsList::Refresh() {
  
  return running;
}

void SettingsList::OnButtonEvent(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_RELEASED) {
    for (int i = 0; i < MAXLISTITEMS; i++) {
      if (apps[i] != Apps::None && object == buttons[i]) {
        app->StartApp(apps[i], DisplayApp::FullRefreshDirections::Up);
        running = false;
        return;
      }
    }
  }
}
