#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <libs/lvgl/src/lv_font/lv_font.h>
#include <libs/lvgl/lvgl.h>
#include "Tile.h"
#include <DisplayApp/DisplayApp.h>


using namespace Pinetime::Applications::Screens;

extern lv_font_t jetbrains_mono_bold_20;

static void event_handler(lv_obj_t * obj, lv_event_t event) {
  Tile* screen = static_cast<Tile *>(obj->user_data);
  uint32_t* eventDataPtr = (uint32_t*) lv_event_get_data();
  uint32_t eventData = *eventDataPtr;
  screen->OnObjectEvent(obj, event, eventData);
}

static const char * btnm_map1[] = {"Meter", "Gauge", "Clock", "\n", "Soft\nversion", "App2", "Brightness", ""};

Tile::Tile(DisplayApp* app) : Screen(app) {
  modal.reset(new Modal(app));
/*
  static lv_point_t valid_pos[] = {{0,0}, {LV_COORD_MIN, LV_COORD_MIN}};
  tileview = lv_tileview_create(lv_scr_act(), NULL);
  lv_tileview_set_valid_positions(tileview, valid_pos, 1);
  lv_tileview_set_edge_flash(tileview, false);

  tile1 = lv_obj_create(tileview, NULL);
  lv_obj_set_pos(tile1, 0, 0);
  lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);
  lv_tileview_add_element(tileview, tile1);
*/
  btnm1 = lv_btnm_create(lv_scr_act(), NULL);
  lv_btnm_set_map(btnm1, btnm_map1);
  lv_obj_set_size(btnm1, LV_HOR_RES, LV_VER_RES);

//  labelRelStyle = const_cast<lv_style_t *>(lv_label_get_style(btnm1, LV_BTNM_STYLE_BTN_REL));
//  labelRelStyle->text.font = &jetbrains_mono_bold_20;
//  labelRelStyle->body.grad_color = labelRelStyle->body.main_color;
//  lv_btnm_set_style(btnm1, LV_BTNM_STYLE_BTN_REL, labelRelStyle);
//
//  labelPrStyle = const_cast<lv_style_t *>(lv_label_get_style(btnm1, LV_BTNM_STYLE_BTN_PR));
//  labelPrStyle->text.font = &jetbrains_mono_bold_20;
//  labelPrStyle->body.grad_color = labelPrStyle->body.shadow.color;



//  lv_btnm_set_style(btnm1, LV_BTNM_STYLE_BTN_PR, labelPrStyle);
//TODO better style handling
//  lv_obj_align(btnm1, tile1, LV_ALIGN_CENTER, 0, 0);
  btnm1->user_data = this;
  lv_obj_set_event_cb(btnm1, event_handler);

/*
  tile2 = lv_obj_create(tileview, NULL);
  lv_obj_set_pos(tile2, 0, LV_VER_RES);
  lv_obj_set_size(tile2, LV_HOR_RES, LV_VER_RES);
  lv_tileview_add_element(tileview, tile2);

  btnm2 = lv_btnm_create(tileview, NULL);
  lv_btnm_set_map(btnm2, btnm_map2);
  lv_obj_align(btnm2, tile2, LV_ALIGN_CENTER, 0, 0);
*/
/*
  tile1 = lv_obj_create(tileview, NULL);
  lv_obj_set_pos(tile1, 0, 0);
  lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);
  lv_tileview_add_element(tileview, tile1);

  btn1 = lv_btn_create(tile1, NULL);
  lv_obj_align(btn1, tile1, LV_ALIGN_CENTER, 0, 0);

  label1 = lv_label_create(btn1, NULL);
  lv_label_set_text(label1, "Button1");
*/
/*
  tile2 = lv_obj_create(tileview, NULL);
  lv_obj_set_pos(tile2, 0, LV_VER_RES);
  lv_obj_set_size(tile2, LV_HOR_RES, LV_VER_RES);
  lv_tileview_add_element(tileview, tile2);

  btn2 = lv_btn_create(tile2, NULL);
  lv_obj_align(btn2, tile2, LV_ALIGN_CENTER, 0, 0);


  label2 = lv_label_create(btn2, NULL);
  lv_label_set_text(label2, "Button2");

  tile3 = lv_obj_create(tileview, NULL);
  lv_obj_set_pos(tile3, 0, LV_VER_RES*2);
  lv_obj_set_size(tile3, LV_HOR_RES, LV_VER_RES);
  lv_tileview_add_element(tileview, tile3);

  btn3 = lv_btn_create(tile3, NULL);
  lv_obj_align(btn3, tile3, LV_ALIGN_CENTER, 0, 0);


  label3 = lv_label_create(btn3, NULL);
  lv_label_set_text(label3, "Button3");
*/
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
  app->StartApp(DisplayApp::Apps::Gauge);
  running = false;
}

