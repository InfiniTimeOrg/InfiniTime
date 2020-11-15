#include "Tab.h"
#include <Version.h>
#include <libraries/log/nrf_log.h>
#include <date/date.h>
#include <lvgl/lvgl.h>
#include "../DisplayApp.h"
#include "components/datetime/DateTimeController.h"
#include "components/gfx/Gfx.h"

using namespace Pinetime::Applications::Screens;

extern lv_font_t jetbrains_mono_bold_20;

//static void event_handler(lv_obj_t * obj, lv_event_t event) {
//  Tile* screen = static_cast<Tile *>(obj->user_data);
//  screen->OnObjectEvent(obj, event);
//}

Tab::Tab(DisplayApp* app, Pinetime::Components::Gfx &gfx) : Screen(app), gfx(gfx) {
/*Create a Tab view object*/
  lv_obj_t *tabview;
  tabview = lv_tabview_create(lv_scr_act(), NULL);

  /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tab 1");
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tab 2");
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Tab 3");


  /*Add content to the tabs*/
  lv_obj_t * label = lv_label_create(tab1, NULL);
  lv_label_set_text(label, "This the first tab\n\n"
                           "If the content\n"
                           "of a tab\n"
                           "become too long\n"
                           "the it \n"
                           "automatically\n"
                           "become\n"
                           "scrollable.");

  label = lv_label_create(tab2, NULL);
  lv_label_set_text(label, "Second tab");

  label = lv_label_create(tab3, NULL);
  lv_label_set_text(label, "Third tab");

}

Tab::~Tab() {
  lv_obj_clean(lv_scr_act());
}

void Tab::Refresh(bool fullRefresh) {

}

void Tab::OnObjectEvent(lv_obj_t *obj, lv_event_t event) {
  if(event == LV_EVENT_CLICKED) {
    NRF_LOG_INFO("Clicked");
  }
  else if(event == LV_EVENT_VALUE_CHANGED) {
    NRF_LOG_INFO("Toggled");
  }
}
