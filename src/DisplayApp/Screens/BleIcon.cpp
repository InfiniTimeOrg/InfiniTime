#include "BleIcon.h"

using namespace Pinetime::Applications::Screens;


extern lv_img_dsc_t os_bt_connected;
extern lv_img_dsc_t os_bt_disconnected;

lv_img_dsc_t *BleIcon::GetIcon(bool isConnected) {
  if(isConnected) return &os_bt_connected;
  else return &os_bt_disconnected;
}