#include "Tile.h"
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;

static void event_handler(lv_obj_t * obj, lv_event_t event) {
  Tile* screen = static_cast<Tile *>(obj->user_data);
  uint32_t* eventDataPtr = (uint32_t*) lv_event_get_data();
  uint32_t eventData = *eventDataPtr;
  screen->OnObjectEvent(obj, event, eventData);
}

Tile::Tile(uint8_t screenID, DisplayApp* app, Controllers::Settings& settingsController, std::array<Applications, 6>& applications) : Screen(app) {
  
  settingsController.SetAppMenu(screenID);
  
  for(int i = 0, appIndex = 0; i < 8; i++) {
    if(i == 3) btnm_map1[i] = "\n";
    else if(i == 7) btnm_map1[i] = "";
    else {
      btnm_map1[i] = applications[appIndex].icon;
      apps[appIndex] = applications[appIndex].application;
      appIndex++;
    }
  }

  btnm1 = lv_btnmatrix_create(lv_scr_act(), nullptr);
  lv_btnmatrix_set_map(btnm1, btnm_map1);
  lv_obj_set_size(btnm1, LV_HOR_RES, LV_VER_RES);

  btnm1->user_data = this;
  lv_obj_set_event_cb(btnm1, event_handler);
}

Tile::~Tile() {
  lv_obj_clean(lv_scr_act());
}

bool Tile::Refresh() {
  return running;
}

void Tile::OnObjectEvent(lv_obj_t *obj, lv_event_t event, uint32_t buttonId) {
  if(event == LV_EVENT_VALUE_CHANGED) {
    app->StartApp(apps[buttonId]);
    running = false;
  }
}

bool Tile::OnButtonPushed() {
  app->StartApp(Apps::Clock);
  running = false;
  return true;
}


