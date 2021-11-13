#include <string>
#include "../DisplayApp.h"
#include "../LittleVgl.h"
#include "libs/QR-Code-generator/c/qrcodegen.h"
#include "components/ble/QrService.h"
#include "Qr.h"

using namespace Pinetime::Applications::Screens;

static void buttonEventHandler(lv_obj_t* obj, lv_event_t event) {
  Qr* screen = static_cast<Qr*>(obj->user_data);
  if (screen->showingQrCode == false) {
    screen->OnButtonEvent(obj, event);
  }
}

Qr::Qr(Pinetime::Applications::DisplayApp* app, Pinetime::Components::LittleVgl& lvgl, Pinetime::Controllers::QrService& qrService)
  : Screen(app), lvgl {lvgl}, qrService(qrService) {
  drawQrList();
  taskRefresh = lv_task_create(RefreshTaskCallback, 100, LV_TASK_PRIO_MID, this);
}

Qr::~Qr() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Qr::Refresh() {
  if (qrList != qrService.getQrList()) {
    qrList = qrService.getQrList();
    drawQrList();
    showingQrCode = false;
  }
}

bool Qr::OnButtonPushed() {
  if (showingQrCode) {
    drawQrList();
    showingQrCode = false;
  } else {
    return false;
  }
  return true;
}

bool Qr::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return true;
}

bool Qr::OnTouchEvent(uint16_t x, uint16_t y) {
  return true;
}

void Qr::drawQrList() {
  lv_obj_clean(lv_scr_act());
  // Set the background to Black
  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0, 0, 0));

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 0, 0);
  lv_obj_set_width(container1, LV_HOR_RES - 15);
  lv_obj_set_height(container1, LV_VER_RES);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* labelBt;

  for (uint8_t i = 0; i < MAXLISTITEMS; i++) {
    if (qrList[i].text != "") {

      itemApps[i] = lv_btn_create(container1, nullptr);
      lv_obj_set_style_local_radius(itemApps[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
      lv_obj_set_style_local_bg_color(itemApps[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
      lv_obj_set_style_local_bg_opa(itemApps[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
      lv_obj_set_width(itemApps[i], LV_HOR_RES - 25);
      lv_obj_set_height(itemApps[i], 52);
      lv_obj_set_event_cb(itemApps[i], buttonEventHandler);
      lv_btn_set_layout(itemApps[i], LV_LAYOUT_ROW_MID);
      itemApps[i]->user_data = this;

      labelBt = lv_label_create(itemApps[i], nullptr);
      lv_label_set_text(labelBt, qrList[i].name.c_str());
    }
  }

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");
}

void Qr::drawQr(std::string qrText) {

  resetScreen();
  showingQrCode = true;

  bool ok =
    qrcodegen_encodeText(qrText.c_str(), tempBuffer, qrcode, qrcodegen_Ecc_HIGH, qrVersionMin, qrVersionMax, qrcodegen_Mask_AUTO, true);
  if (ok) {
    qrSize = qrcodegen_getSize(qrcode);
    qrModuleSize = LV_HOR_RES_MAX / (qrSize + 2 * border);
    offset = (LV_HOR_RES_MAX - (qrSize + 2 * border) * qrModuleSize) / 2;

    std::fill(colorBuffer, colorBuffer + colorBufferSize, LV_COLOR_WHITE);

    for (int16_t y = 0; y < qrSize + border * 2; y++) {
      for (int16_t x = 0; x < (qrSize + border * 2); x++) {
        if (!qrcodegen_getModule(qrcode, x - border, y - border)) {
          area.x1 = qrModuleSize * x + offset;
          area.y1 = qrModuleSize * y + offset;
          area.x2 = qrModuleSize * (x + 1) + offset - 1;
          area.y2 = qrModuleSize * (y + 1) + offset - 1;
          lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
          lvgl.FlushDisplay(&area, colorBuffer);
        }
      }
    }
  }
}

void Qr::resetScreen() {
  std::fill(colorBuffer, colorBuffer + colorBufferSize, LV_COLOR_BLACK);
  for (uint8_t y = 0; y < (LV_VER_RES_MAX / maxPixelsPerQrModuleSide); y++) {
    for (uint8_t x = 0; x < (LV_HOR_RES_MAX / maxPixelsPerQrModuleSide); x++) {
      area.x1 = maxPixelsPerQrModuleSide * x;
      area.y1 = maxPixelsPerQrModuleSide * y;
      area.x2 = maxPixelsPerQrModuleSide * (x + 1) - 1;
      area.y2 = maxPixelsPerQrModuleSide * (y + 1) - 1;
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
      lvgl.FlushDisplay(&area, colorBuffer);
    }
  }
}

void Qr::OnButtonEvent(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_RELEASED) {
    for (int i = 0; i < MAXLISTITEMS; i++) {
      if (qrList[i].text != "" && object == itemApps[i]) {
        drawQr(qrList[i].text);
        return;
      }
    }
  }
}
