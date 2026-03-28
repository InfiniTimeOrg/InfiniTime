#include "Home.h"
#include "components/ble/HomeService.h"
#include "displayapp/DisplayApp.h"
#include <libraries/log/nrf_log.h>

using namespace Pinetime::Applications::Screens;

static void RequestUpdateTaskCallback(lv_task_t* task) {
  static_cast<Home*>(task->user_data)->RequestUpdate();
}

static void event_handler(lv_obj_t* obj, lv_event_t event) {
  static_cast<Home*>(obj->parent->user_data)->OnObjectEvent(obj, event);
}

Home::Home(Pinetime::Controllers::HomeService& home, DisplayApp* app) : homeService(home), app(app) {
  is_connected = home.OnOpened();

  container = lv_obj_create(lv_scr_act(), lv_scr_act());
  container->user_data = this;

  label_status = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_status, is_connected ? "loading" : "not connected");
  lv_obj_align(label_status, nullptr, LV_ALIGN_CENTER, 0, 0);

  if (is_connected) {
    taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
    taskRequestUpdate = lv_task_create(RequestUpdateTaskCallback, 1000, LV_TASK_PRIO_MID, this);
  }
}

Home::~Home() {
  lv_task_del(taskRefresh);
  lv_task_del(taskRequestUpdate);
  lv_obj_clean(lv_scr_act());

  if (is_connected)
    homeService.OnClosed();
}

void Home::Refresh() {
  if (updated_at >= homeService.DataUpdateTime())
    return; // No need to refresh, we don't have new data

  if (current_screen == homeService.CurrentScreen().index) {
    updated_at = homeService.DataUpdateTime();

    lv_obj_set_hidden(label_status, true);

    ShowScreen(homeService.CurrentScreen());
  }
}

void Home::OnObjectEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_SHORT_CLICKED) {
    size_t componentId = (size_t) obj->user_data;

    homeService.OnPressed(current_screen, (uint8_t) componentId);
  }
}

bool Pinetime::Applications::Screens::Home::OnTouchEvent(TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeRight:
      if (current_screen == 0)
        return false;

      current_screen--;
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::RightAnim);
      break;

    case TouchEvents::SwipeLeft:
      if (current_screen == homeService.NumScreens() - 1)
        return false;

      current_screen++;
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::LeftAnim);
      break;

    default:
      return false;
  }

  lv_obj_set_hidden(label_status, false);
  homeService.OnViewScreen(current_screen);
  return true;
}

void Pinetime::Applications::Screens::Home::ShowScreen(const Pinetime::Controllers::HomeService::Screen& screen) {
  lv_obj_clean(container);

  const int margin = 5;

  auto width = lv_obj_get_width_grid(lv_scr_act(), screen.cols, 1);
  auto height = lv_obj_get_height_grid(lv_scr_act(), screen.rows, 1);

  for (size_t i = 0; i < screen.components.size(); i++) {
    auto& comp = screen.components[i];

    switch (comp.type) {
      case Pinetime::Controllers::HomeService::ComponentType::Button: {
        lv_obj_t* btn = lv_btn_create(container, nullptr);
        btn->user_data = (lv_obj_user_data_t) i;
        lv_obj_set_event_cb(btn, event_handler);
        lv_obj_set_size(btn, (width * comp.w) - margin * 2, (height * comp.h) - margin * 2);
        lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(255, 0, 0));
        lv_obj_align(btn, nullptr, LV_ALIGN_IN_TOP_LEFT, comp.x * width + margin, comp.y * height + margin);

        lv_obj_t* lbl = lv_label_create(btn, nullptr);
        lv_label_set_text(lbl, comp.label.get());

        break;
      }

      case Pinetime::Controllers::HomeService::ComponentType::Label: {
        lv_obj_t* cont = lv_obj_create(container, lv_scr_act());
        lv_obj_set_size(cont, (width * comp.w) - margin * 2, (height * comp.h) - margin * 2);
        lv_obj_align(cont, nullptr, LV_ALIGN_IN_TOP_LEFT, comp.x * width + margin, comp.y * height + margin);

        lv_obj_t* lbl = lv_label_create(cont, nullptr);
        lv_label_set_text(lbl, comp.label.get());
        lv_obj_align(lbl, nullptr, LV_ALIGN_CENTER, 0, 0);

        break;
      }

      default:
        break;
    }
  }
}

void Pinetime::Applications::Screens::Home::RequestUpdate() {
  homeService.OnViewScreen(current_screen);
}
