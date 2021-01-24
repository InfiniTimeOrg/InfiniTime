#include "Notifications.h"
#include <displayapp/DisplayApp.h>
#include "components/ble/MusicService.h"

using namespace Pinetime::Applications::Screens;

Notifications::Notifications(DisplayApp *app,
                             Pinetime::Controllers::NotificationManager &notificationManager,
                             Pinetime::Controllers::AlertNotificationService& alertNotificationService,
                             Modes mode) :
        Screen(app), notificationManager{notificationManager}, alertNotificationService{alertNotificationService}, mode{mode} {
  notificationManager.ClearNewNotificationFlag();
  auto notification = notificationManager.GetLastNotification();
  if(notification.valid) {
    currentId = notification.id;
    currentItem.reset(new NotificationItem("\nNotification",
                                           notification.message.data(),
                                           notification.index,
                                           notification.category,
                                           notificationManager.NbNotifications(),
                                           mode,
                                           alertNotificationService));
    validDisplay = true;
  } else {
    currentItem.reset(new NotificationItem("\nNotification",
                                           "No notification to display",
                                           0,
                                           notification.category,
                                           notificationManager.NbNotifications(),
                                           Modes::Preview,
                                           alertNotificationService));
  }

  if(mode == Modes::Preview) {
    static lv_style_t style_line;
    lv_style_copy(&style_line, &lv_style_plain);
    style_line.line.color = LV_COLOR_WHITE;
    style_line.line.width = 3;
    style_line.line.rounded = 0;


    timeoutLine = lv_line_create(lv_scr_act(), nullptr);
    lv_line_set_style(timeoutLine, LV_LINE_STYLE_MAIN, &style_line);
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
      currentItem.reset(new NotificationItem("\nNotification",
                                             previousNotification.message.data(),
                                             previousNotification.index,
                                             previousNotification.category,
                                             notificationManager.NbNotifications(),
                                             mode,
                                             alertNotificationService));
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
      currentItem.reset(new NotificationItem("\nNotification",
                                             nextNotification.message.data(),
                                             nextNotification.index,
                                             nextNotification.category,
                                             notificationManager.NbNotifications(),
                                             mode,
                                             alertNotificationService));
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

namespace {
  static void AcceptIncomingCallEventHandler(lv_obj_t *obj, lv_event_t event) {
    auto* item = static_cast<Notifications::NotificationItem *>(obj->user_data);
    item->OnAcceptIncomingCall(event);
  }

  static void RejectIncomingCallEventHandler(lv_obj_t *obj, lv_event_t event) {
    auto* item = static_cast<Notifications::NotificationItem *>(obj->user_data);
    item->OnRejectIncomingCall(event);
  }
}

Notifications::NotificationItem::NotificationItem(const char *title,
                                                  const char *msg,
                                                  uint8_t notifNr,
                                                  Controllers::NotificationManager::Categories category,
                                                  uint8_t notifNb,
                                                  Modes mode,
                                                  Pinetime::Controllers::AlertNotificationService& alertNotificationService)
        : notifNr{notifNr}, notifNb{notifNb}, mode{mode}, alertNotificationService{alertNotificationService} {
  container1 = lv_cont_create(lv_scr_act(), nullptr);
  static lv_style_t contStyle;
  lv_style_copy(&contStyle, lv_cont_get_style(container1, LV_CONT_STYLE_MAIN));
  contStyle.body.padding.inner = 20;
  lv_cont_set_style(container1, LV_CONT_STYLE_MAIN, &contStyle);
  lv_obj_set_width(container1, LV_HOR_RES);
  lv_obj_set_height(container1, LV_VER_RES);
  lv_obj_set_pos(container1, 0, 0);
  lv_cont_set_layout(container1, LV_LAYOUT_OFF);
  lv_cont_set_fit2(container1, LV_FIT_FLOOD, LV_FIT_FLOOD);

  t1 = lv_label_create(container1, nullptr);
  static lv_style_t titleStyle;
  static lv_style_t textStyle;
  static lv_style_t bottomStyle;
  lv_style_copy(&titleStyle, lv_label_get_style(t1, LV_LABEL_STYLE_MAIN));
  lv_style_copy(&textStyle, lv_label_get_style(t1, LV_LABEL_STYLE_MAIN));
  lv_style_copy(&bottomStyle, lv_label_get_style(t1, LV_LABEL_STYLE_MAIN));
  titleStyle.body.padding.inner = 5;
  titleStyle.body.grad_color = LV_COLOR_GRAY;
  titleStyle.body.main_color = LV_COLOR_GRAY;
  titleStyle.body.radius = 20;
  textStyle.body.border.part = LV_BORDER_NONE;
  textStyle.body.padding.inner = 5;

  bottomStyle.body.main_color = LV_COLOR_GREEN;
  bottomStyle.body.grad_color = LV_COLOR_GREEN;
  bottomStyle.body.border.part = LV_BORDER_TOP;
  bottomStyle.body.border.color = LV_COLOR_RED;

  lv_label_set_style(t1, LV_LABEL_STYLE_MAIN, &titleStyle);
  lv_label_set_long_mode(t1, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(t1, true);
  lv_obj_set_width(t1, LV_HOR_RES - (titleStyle.body.padding.left + titleStyle.body.padding.right));
  lv_label_set_text(t1, title);
  static constexpr int16_t offscreenOffset = -20 ;
  lv_obj_set_pos(t1, titleStyle.body.padding.left, offscreenOffset);

  auto titleHeight = lv_obj_get_height(t1);

  switch(category) {
    default: {
      l1 = lv_label_create(container1, nullptr);
      lv_label_set_style(l1, LV_LABEL_STYLE_MAIN, &textStyle);
      lv_obj_set_pos(l1, textStyle.body.padding.left,
                     titleHeight + offscreenOffset + textStyle.body.padding.bottom +
                     textStyle.body.padding.top);

      lv_label_set_long_mode(l1, LV_LABEL_LONG_BREAK);
      lv_label_set_body_draw(l1, true);
      lv_obj_set_width(l1, LV_HOR_RES - (textStyle.body.padding.left + textStyle.body.padding.right));
      lv_label_set_text(l1, msg);
    }
    break;
    case Controllers::NotificationManager::Categories::IncomingCall: {
      l1 = lv_label_create(container1, nullptr);
      lv_label_set_style(l1, LV_LABEL_STYLE_MAIN, &textStyle);
      lv_obj_set_pos(l1, textStyle.body.padding.left,
                     titleHeight + offscreenOffset + textStyle.body.padding.bottom +
                     textStyle.body.padding.top);

      lv_label_set_long_mode(l1, LV_LABEL_LONG_BREAK);
      lv_label_set_body_draw(l1, true);
      lv_obj_set_width(l1, LV_HOR_RES - (textStyle.body.padding.left + textStyle.body.padding.right));
      lv_label_set_text(l1, "Incoming call from ");
      auto l1Height = lv_obj_get_height(l1);

      l2 = lv_label_create(container1, nullptr);
      lv_label_set_style(l2, LV_LABEL_STYLE_MAIN, &textStyle);
      lv_obj_set_pos(l2, textStyle.body.padding.left,
                     titleHeight + l1Height + offscreenOffset + (textStyle.body.padding.bottom*2) +
                         (textStyle.body.padding.top*2));
      lv_label_set_long_mode(l2, LV_LABEL_LONG_BREAK);
      lv_label_set_body_draw(l2, true);
      lv_obj_set_width(l2, LV_HOR_RES - (textStyle.body.padding.left + textStyle.body.padding.right));
      lv_label_set_text(l2, msg);

      bt_accept = lv_btn_create(container1, nullptr);
      lv_obj_align(bt_accept, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, -20);
      bt_accept->user_data = this;
      lv_obj_set_event_cb(bt_accept, AcceptIncomingCallEventHandler);

      label_accept = lv_label_create(bt_accept, nullptr);
      lv_label_set_text(label_accept, "Accept");

      bt_reject = lv_btn_create(container1, nullptr);
      lv_obj_align(bt_reject, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, -20);
      bt_reject->user_data = this;
      lv_obj_set_event_cb(bt_reject, RejectIncomingCallEventHandler);
      label_reject = lv_label_create(bt_reject, nullptr);
      lv_label_set_text(label_reject, "Reject");
    }
  }

  if(mode == Modes::Normal) {
    if(notifNr < notifNb) {
      bottomPlaceholder = lv_label_create(container1, nullptr);
      lv_label_set_style(bottomPlaceholder, LV_LABEL_STYLE_MAIN, &titleStyle);
      lv_label_set_long_mode(bottomPlaceholder, LV_LABEL_LONG_BREAK);
      lv_label_set_body_draw(bottomPlaceholder, true);
      lv_obj_set_width(bottomPlaceholder, LV_HOR_RES - (titleStyle.body.padding.left + titleStyle.body.padding.right));
      lv_label_set_text(bottomPlaceholder, " ");
      lv_obj_set_pos(bottomPlaceholder, titleStyle.body.padding.left, LV_VER_RES - 5);
    }
  }
}

void Notifications::NotificationItem::OnAcceptIncomingCall(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) return;

  alertNotificationService.AcceptIncomingCall();
}

void Notifications::NotificationItem::OnRejectIncomingCall(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) return;

  alertNotificationService.RejectIncomingCall();
}

Notifications::NotificationItem::~NotificationItem() {
  lv_obj_clean(lv_scr_act());
}
