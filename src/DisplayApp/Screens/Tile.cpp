#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <libs/lvgl/src/lv_font/lv_font.h>
#include <libs/lvgl/lvgl.h>
#include "Tile.h"
#include <DisplayApp/DisplayApp.h>
#include "Symbols.h"
#include "../../Version.h"

using namespace Pinetime::Applications::Screens;

extern lv_font_t jetbrains_mono_bold_20;

static void event_handler(lv_obj_t * obj, lv_event_t event) {
  Tile* screen = static_cast<Tile *>(obj->user_data);
  uint32_t* eventDataPtr = (uint32_t*) lv_event_get_data();
  uint32_t eventData = *eventDataPtr;
  screen->OnObjectEvent(obj, event, eventData);
}

static const char * btnm_map1[] = {Symbols::heartBeat, Symbols::shoe, Symbols::clock, "\n", Symbols::info, Symbols::list, Symbols::sun, ""};

Tile::Tile(DisplayApp* app) : Screen(app) {
  modal.reset(new Modal(app));

  btnm1 = lv_btnm_create(lv_scr_act(), NULL);
  lv_btnm_set_map(btnm1, btnm_map1);
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
  auto* tile = static_cast<Tile*>(obj->user_data);
  if(event == LV_EVENT_VALUE_CHANGED) {
    switch(buttonId) {
      case 0:
        tile->StartMeterApp();
        break;
      case 1:
        tile->StartGaugeApp();
        break;
      case 2:
        tile->StartClockApp();
        break;
      case 3:
        char versionStr[20];
        sprintf(versionStr, "VERSION: %d.%d.%d", Version::Major(), Version::Minor(), Version::Patch());
        modal->Show(versionStr);
        break;
      case 4:
        tile->StartSysInfoApp();
        break;
      case 5:
        tile->StartBrightnessApp();

        break;
    }
    clickCount++;
  }
}

bool Tile::OnButtonPushed() {
  app->StartApp(DisplayApp::Apps::Clock);
  running = false;
  return true;
}

void Tile::StartClockApp() {
  app->StartApp(DisplayApp::Apps::Clock);
  running = false;
}

void Tile::StartSysInfoApp() {
  app->StartApp(DisplayApp::Apps::SysInfo);
  running = false;
}

void Tile::StartBrightnessApp() {
  app->StartApp(DisplayApp::Apps::Brightness);
  running = false;
}

void Tile::StartMeterApp() {
  app->StartApp(DisplayApp::Apps::Meter);
  running = false;
}

void Tile::StartGaugeApp() {
  app->StartApp(DisplayApp::Apps::Music);
  running = false;
}

