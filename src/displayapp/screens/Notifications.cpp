#include "Notifications.h"
#include <displayapp/DisplayApp.h>

using namespace Pinetime::Applications::Screens;

Notifications::Notifications(DisplayApp *app, Pinetime::Controllers::NotificationManager &notificationManager, Modes mode) :
        Screen(app), notificationManager{notificationManager}, mode{mode} {
  notificationManager.ClearNewNotificationFlag();
  auto notification = notificationManager.GetLastNotification();
  if(notification.valid) {
    currentId = notification.id;
    currentItem.reset(new NotificationItem("\nNotification", notification.message.data(), notification.index, notificationManager.NbNotifications(), mode));
    validDisplay = true;
  } else {
    currentItem.reset(new NotificationItem("\nNotification", "No notification to display", 0, notificationManager.NbNotifications(), Modes::Preview));
  }

  if(mode == Modes::Preview) {
    
    timeoutLine = lv_line_create(lv_scr_act(), nullptr);

    lv_obj_set_style_local_line_width(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_line_rounded(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);

    lv_line_set_points(timeoutLine, timeoutLinePoints, 2);
    timeoutTickCountStart = xTaskGetTickCount();
    timeoutTickCountEnd = timeoutTickCountStart + (5*1024);
  }
}

Notifications::~Notifications() {
  lv_obj_clean(lv_scr_act());
}

bool Notifications::Refresh() {
  if (mode == Modes::Preview) {
    auto tick = xTaskGetTickCount();
    int32_t pos = 240 - ((tick - timeoutTickCountStart) / ((timeoutTickCountEnd - timeoutTickCountStart) / 240));
    if (pos < 0)
      running = false;

    timeoutLinePoints[1].x = pos;
    lv_line_set_points(timeoutLine, timeoutLinePoints, 2);

    if (!running) {
      // Start clock app when exiting this one
      app->StartApp(Apps::Clock);
    }
  }

  return running;
}

bool Notifications::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case Pinetime::Applications::TouchEvents::SwipeUp: {
      Controllers::NotificationManager::Notification previousNotification;
      if(validDisplay)
        previousNotification = notificationManager.GetPrevious(currentId);
      else
        previousNotification = notificationManager.GetLastNotification();

      if (!previousNotification.valid) return true;

      validDisplay = true;
      currentId = previousNotification.id;
      currentItem.reset(nullptr);
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::Up);
      currentItem.reset(new NotificationItem("\nNotification", previousNotification.message.data(),  previousNotification.index, notificationManager.NbNotifications(), mode));
    }
      return true;
    case Pinetime::Applications::TouchEvents::SwipeDown: {
      Controllers::NotificationManager::Notification nextNotification;
      if(validDisplay)
        nextNotification = notificationManager.GetNext(currentId);
      else
        nextNotification = notificationManager.GetLastNotification();

      if (!nextNotification.valid) return true;

      validDisplay = true;
      currentId = nextNotification.id;
      currentItem.reset(nullptr);
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::Down);
      currentItem.reset(new NotificationItem("\nNotification", nextNotification.message.data(),  nextNotification.index, notificationManager.NbNotifications(), mode));
    }
      return true;
    default:
      return false;
  }
}


bool Notifications::OnButtonPushed() {
  running = false;
  return true;
}


Notifications::NotificationItem::NotificationItem(const char *title, const char *msg, uint8_t notifNr, uint8_t notifNb, Modes mode)
        : notifNr{notifNr}, notifNb{notifNb}, mode{mode} {

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), NULL);

  lv_obj_set_style_local_bg_color(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x222222));
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 0, 50);
  lv_obj_set_width(container1, 240);
  lv_obj_set_height(container1, 190);
  
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* alert_count = lv_label_create(lv_scr_act(), nullptr);    
  lv_label_set_text_fmt(alert_count, "%i/%i", notifNr, notifNb);
  lv_obj_align(alert_count, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 16);

  lv_obj_t* alert_type = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(alert_type, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x888888));   
  lv_label_set_text(alert_type, title);
  lv_obj_align(alert_type, NULL, LV_ALIGN_IN_TOP_LEFT, 0, -4); 

  lv_obj_t* alert_subject = lv_label_create(container1, nullptr);
  lv_obj_set_style_local_text_color(alert_subject, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_long_mode(alert_subject, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(alert_subject, LV_HOR_RES - 20);    
  lv_label_set_text(alert_subject, msg);
  //lv_obj_align(alert_subject, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 50);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");
  
}


Notifications::NotificationItem::~NotificationItem() {
  lv_obj_clean(lv_scr_act());
}
