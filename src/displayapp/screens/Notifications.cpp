#include "Notifications.h"
#include <displayapp/DisplayApp.h>
#include "components/ble/MusicService.h"
#include "components/ble/AlertNotificationService.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;
extern const lv_font_t jetbrains_mono_extrabold_compressed;
extern const lv_font_t jetbrains_mono_bold_20;

Notifications::Notifications(DisplayApp* app,
                             Pinetime::Controllers::NotificationManager& notificationManager,
                             Pinetime::Controllers::AlertNotificationService& alertNotificationService,
                             Pinetime::Controllers::MotorController& motorController,
                             Modes mode)
  : Screen(app), notificationManager {notificationManager}, alertNotificationService {alertNotificationService}, mode {mode} {
  notificationManager.ClearNewNotificationFlag();
  auto notification = notificationManager.GetLastNotification();
  if (notification.valid) {
    currentId = notification.id;
    currentItem = std::make_unique<NotificationItem>(notification.Title(),
                                                     notification.Message(),
                                                     notification.index,
                                                     notification.category,
                                                     notificationManager.NbNotifications(),
                                                     mode,
                                                     alertNotificationService);
    validDisplay = true;
  } else {
    currentItem = std::make_unique<NotificationItem>("Notification",
                                                     "No notification to display",
                                                     0,
                                                     notification.category,
                                                     notificationManager.NbNotifications(),
                                                     Modes::Preview,
                                                     alertNotificationService);
  }

  if (mode == Modes::Preview) {
    if (notification.category == Controllers::NotificationManager::Categories::IncomingCall) {
      motorController.StartRinging();
    } else {
      motorController.RunForDuration(35);
      timeoutLine = lv_line_create(lv_scr_act());

      lv_obj_set_style_line_width(timeoutLine, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_line_color(timeoutLine, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_line_rounded(timeoutLine, true, LV_PART_MAIN | LV_STATE_DEFAULT);

      lv_line_set_points(timeoutLine, timeoutLinePoints, 2);
      timeoutTickCountStart = xTaskGetTickCount();
      timeoutTickCountEnd = timeoutTickCountStart + (5 * 1024);
    }
  }

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, this);
}

Notifications::~Notifications() {
  lv_timer_del(taskRefresh);
  // make sure we stop any vibrations before exiting
  Controllers::MotorController::StopRinging();
  lv_obj_clean(lv_scr_act());
}

void Notifications::Refresh() {
  if (mode == Modes::Preview && timeoutLine != nullptr) {
    auto tick = xTaskGetTickCount();
    int32_t pos = 240 - ((tick - timeoutTickCountStart) / ((timeoutTickCountEnd - timeoutTickCountStart) / 240));
    if (pos < 0)
      running = false;

    timeoutLinePoints[1].x = pos;
    lv_line_set_points(timeoutLine, timeoutLinePoints, 2);
  }
}

bool Notifications::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (mode != Modes::Normal) {
    return false;
  }

  switch (event) {
    case Pinetime::Applications::TouchEvents::SwipeDown: {
      Controllers::NotificationManager::Notification previousNotification;
      if (validDisplay)
        previousNotification = notificationManager.GetPrevious(currentId);
      else
        previousNotification = notificationManager.GetLastNotification();

      if (!previousNotification.valid)
        return true;

      validDisplay = true;
      currentId = previousNotification.id;
      currentItem.reset(nullptr);
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::Down);
      currentItem = std::make_unique<NotificationItem>(previousNotification.Title(),
                                                       previousNotification.Message(),
                                                       previousNotification.index,
                                                       previousNotification.category,
                                                       notificationManager.NbNotifications(),
                                                       mode,
                                                       alertNotificationService);
    }
      return true;
    case Pinetime::Applications::TouchEvents::SwipeUp: {
      Controllers::NotificationManager::Notification nextNotification;
      if (validDisplay)
        nextNotification = notificationManager.GetNext(currentId);
      else
        nextNotification = notificationManager.GetLastNotification();

      if (!nextNotification.valid) {
        running = false;
        return false;
      }

      validDisplay = true;
      currentId = nextNotification.id;
      currentItem.reset(nullptr);
      app->SetFullRefresh(DisplayApp::FullRefreshDirections::Up);
      currentItem = std::make_unique<NotificationItem>(nextNotification.Title(),
                                                       nextNotification.Message(),
                                                       nextNotification.index,
                                                       nextNotification.category,
                                                       notificationManager.NbNotifications(),
                                                       mode,
                                                       alertNotificationService);
    }
      return true;
    default:
      return false;
  }
}

namespace {
  void CallEventHandler(lv_event_t* event) {
    auto* item = static_cast<Notifications::NotificationItem*>(lv_event_get_user_data(event));
    item->OnCallButtonEvent(lv_event_get_target(event), event);
  }
}

Notifications::NotificationItem::NotificationItem(const char* title,
                                                  const char* msg,
                                                  uint8_t notifNr,
                                                  Controllers::NotificationManager::Categories category,
                                                  uint8_t notifNb,
                                                  Modes mode,
                                                  Pinetime::Controllers::AlertNotificationService& alertNotificationService)
  : mode {mode}, alertNotificationService {alertNotificationService} {
  lv_obj_t* container1 = lv_obj_create(lv_scr_act());

  lv_obj_set_style_bg_color(container1, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_all(container1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_gap(container1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(container1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_pos(container1, 0, 50);
  lv_obj_set_size(container1, LV_HOR_RES, 190);

  lv_obj_set_layout(container1, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(container1, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(container1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

  lv_obj_t* alert_count = lv_label_create(lv_scr_act());
  lv_label_set_text_fmt(alert_count, "%i/%i", notifNr, notifNb);
  lv_obj_align(alert_count, LV_ALIGN_TOP_RIGHT, 0, 16);

  lv_obj_t* alert_type = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(alert_type, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
  if (title == nullptr)
    title = "Notification";
  char* pchar;
  pchar = strchr(title, '\n');
  while (pchar != nullptr) {
    *pchar = ' ';
    pchar = strchr(pchar + 1, '\n');
  }
  lv_label_set_text(alert_type, title);
  lv_label_set_long_mode(alert_type, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(alert_type, 180);
  lv_obj_align(alert_type, LV_ALIGN_TOP_LEFT, 0, 16);

  /////////
  switch (category) {
    default: {
      lv_obj_t* alert_subject = lv_label_create(container1);
      lv_obj_set_style_text_color(alert_subject, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_label_set_long_mode(alert_subject, LV_LABEL_LONG_WRAP);
      lv_obj_set_width(alert_subject, LV_HOR_RES - 20);
      lv_label_set_text(alert_subject, msg);
    } break;
    case Controllers::NotificationManager::Categories::IncomingCall: {
      lv_obj_set_height(container1, 108);
      lv_obj_t* alert_subject = lv_label_create(container1);
      lv_obj_set_style_text_color(alert_subject, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_label_set_long_mode(alert_subject, LV_LABEL_LONG_WRAP);
      lv_obj_set_width(alert_subject, LV_HOR_RES - 20);
      lv_label_set_text(alert_subject, "Incoming call from");

      lv_obj_t* alert_caller = lv_label_create(container1);
      lv_obj_align_to(alert_caller, alert_subject, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
      lv_label_set_long_mode(alert_caller, LV_LABEL_LONG_WRAP);
      lv_obj_set_width(alert_caller, LV_HOR_RES - 20);
      lv_label_set_text(alert_caller, msg);

      bt_accept = lv_btn_create(lv_scr_act());
      bt_accept->user_data = this;
      lv_obj_add_event_cb(bt_accept, CallEventHandler, LV_EVENT_ALL, bt_accept->user_data);
      lv_obj_set_size(bt_accept, 76, 76);
      lv_obj_align(bt_accept, LV_ALIGN_BOTTOM_LEFT, 0, 0);
      label_accept = lv_label_create(bt_accept);
      lv_label_set_text(label_accept, Symbols::phone);
      lv_obj_center(label_accept);
      lv_obj_set_style_bg_color(bt_accept, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);

      bt_reject = lv_btn_create(lv_scr_act());
      bt_reject->user_data = this;
      lv_obj_add_event_cb(bt_reject, CallEventHandler, LV_EVENT_ALL, bt_reject->user_data);
      lv_obj_set_size(bt_reject, 76, 76);
      lv_obj_align(bt_reject, LV_ALIGN_BOTTOM_MID, 0, 0);
      label_reject = lv_label_create(bt_reject);
      lv_label_set_text(label_reject, Symbols::phoneSlash);
      lv_obj_center(label_reject);
      lv_obj_set_style_bg_color(bt_reject, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN | LV_STATE_DEFAULT);

      bt_mute = lv_btn_create(lv_scr_act());
      bt_mute->user_data = this;
      lv_obj_add_event_cb(bt_mute, CallEventHandler, LV_EVENT_ALL, bt_mute->user_data);
      lv_obj_set_size(bt_mute, 76, 76);
      lv_obj_align(bt_mute, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
      label_mute = lv_label_create(bt_mute);
      lv_label_set_text(label_mute, Symbols::volumMute);
      lv_obj_center(label_mute);
      lv_obj_set_style_bg_color(bt_mute, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT);
    } break;
  }

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");
}

void Notifications::NotificationItem::OnCallButtonEvent(lv_obj_t* obj, lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
    return;
  }

  Controllers::MotorController::StopRinging();

  if (obj == bt_accept) {
    alertNotificationService.AcceptIncomingCall();
  } else if (obj == bt_reject) {
    alertNotificationService.RejectIncomingCall();
  } else if (obj == bt_mute) {
    alertNotificationService.MuteIncomingCall();
  }

  running = false;
}

Notifications::NotificationItem::~NotificationItem() {
  lv_obj_clean(lv_scr_act());
}
