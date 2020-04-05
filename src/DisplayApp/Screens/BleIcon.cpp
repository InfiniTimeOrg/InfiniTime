#include "BleIcon.h"

using namespace Pinetime::Applications::Screens;


extern lv_img_dsc_t ck_os_bt_connected;
extern lv_img_dsc_t ck_os_bt_disconnected;

lv_img_dsc_t *BleIcon::GetIcon(bool isConnected) {
  if(isConnected) return &ck_os_bt_connected;
  else return &ck_os_bt_disconnected;
}