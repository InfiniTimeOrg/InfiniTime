#include "PawnList.h"

#include "components/fs/FS.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

#define PAWN_EXTENSION ".amx"
constexpr int appsPerScreen = 4;

void ButtonCallback(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    PawnList::AppListing* data = static_cast<PawnList::AppListing*>(obj->user_data);
    data->pawnList->ButtonClickedHandler(data->index);
  }
}

PawnList::PawnList(Pinetime::Controllers::FS& filesystem, Applications::DisplayApp* displayApp) : displayApp(displayApp) {
  lfs_dir_t dir;
  lfs_info info;
  if (filesystem.DirOpen("/apps", &dir) == LFS_ERR_OK) {
    int i = 0;

    while (filesystem.DirRead(&dir, &info)) {
      if (info.type == LFS_TYPE_DIR)
        continue;

      size_t len = strlen(info.name);
      if (len < sizeof(PAWN_EXTENSION) || strcmp(info.name + len - (sizeof(PAWN_EXTENSION) - 1), PAWN_EXTENSION) != 0)
        continue;

      pawnApps.push_back(AppListing {
        .pawnList = this,
        .name = std::string(info.name, len - (sizeof(PAWN_EXTENSION) - 1)), // Remove extension
        .index = i++,
      });
    }

    filesystem.DirClose(&dir);
  }

  ShowScreen();
}

PawnList::~PawnList() {
  lv_obj_clean(lv_scr_act());
}

void PawnList::ButtonClickedHandler(int index) {
  char path[LFS_NAME_MAX];
  snprintf(path, sizeof(path), "/apps/%s" PAWN_EXTENSION, pawnApps[index].name.c_str());
  displayApp->StartPawnApp(path, Applications::DisplayApp::FullRefreshDirections::LeftAnim);
}

bool PawnList::OnTouchEvent(TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeUp:
      if (currentScreen < pawnApps.size() / appsPerScreen) {
        currentScreen++;
        displayApp->SetFullRefresh(DisplayApp::FullRefreshDirections::Up);
        ShowScreen();
        return true;
      }
      break;

    case TouchEvents::SwipeDown:
      if (currentScreen > 0) {
        currentScreen--;
        displayApp->SetFullRefresh(DisplayApp::FullRefreshDirections::Down);
        ShowScreen();
        return true;
      }
      break;

    default:
      return false;
  }

  return false;
}

void PawnList::ShowScreen() {
  int relCount = 0;

  lv_obj_clean(lv_scr_act());

  for (size_t i = appsPerScreen * currentScreen; i < pawnApps.size(); i++) {
    lv_obj_t* btn = lv_btn_create(lv_scr_act(), nullptr);
    lv_obj_set_size(btn, 240, 240 / appsPerScreen);
    lv_obj_set_pos(btn, 0, relCount * 240 / appsPerScreen);
    lv_obj_set_event_cb(btn, ButtonCallback);
    lv_obj_set_user_data(btn, &pawnApps[i]);

    lv_obj_t* label = lv_label_create(btn, nullptr);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SROLL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text_static(label, pawnApps[i].name.c_str());
    lv_obj_set_style_local_pad_hor(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_width(label, 240);

    relCount++;
  }
}
