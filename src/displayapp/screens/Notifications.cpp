#include "Notifications.h"
#include <displayapp/DisplayApp.h>
#include "components/ble/MusicService.h"
#include "components/ble/AlertNotificationService.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;
extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;

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
      timeoutLine = lv_line_create(lv_scr_act(), nullptr);

      lv_obj_set_style_local_line_width(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
      lv_obj_set_style_local_line_color(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_obj_set_style_local_line_rounded(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);

      lv_line_set_points(timeoutLine, timeoutLinePoints, 2);
      timeoutTickCountStart = xTaskGetTickCount();
      timeoutTickCountEnd = timeoutTickCountStart + (5 * 1024);
    }
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Notifications::~Notifications() {
  lv_task_del(taskRefresh);
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
    case Pinetime::Applications::TouchEvents::LongTap: {
      // notificationManager.ToggleVibrations();
      return true;
    }
    default:
      return false;
  }
}

namespace {
  void CallEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* item = static_cast<Notifications::NotificationItem*>(obj->user_data);
    item->OnCallButtonEvent(obj, event);
  }
}

Notifications::NotificationItem::NotificationItem(const char* title,
                                                  const char* msg,
                                                  uint8_t notifNr,
                                                  Controllers::NotificationManager::Categories category,
                                                  uint8_t notifNb,
                                                  Modes mode,
                                                  Pinetime::Controllers::AlertNotificationService& alertNotificationService)
  : notifNr {notifNr}, notifNb {notifNb}, mode {mode}, alertNotificationService {alertNotificationService} {
  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), NULL);

  lv_obj_set_style_local_bg_color(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x222222));
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 0, 50);
  lv_obj_set_size(container1, LV_HOR_RES, 190);

  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);
  lv_cont_set_fit(container1, LV_FIT_NONE);

  lv_obj_t* alert_count = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(alert_count, "%i/%i", notifNr, notifNb);
  lv_obj_align(alert_count, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 16);

  lv_obj_t* alert_type = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(alert_type, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x888888));
  if (title == nullptr)
    title = "Notification";
  char* pchar;
  pchar = strchr(title, '\n');
  while (pchar != nullptr) {
    *pchar = ' ';
    pchar = strchr(pchar + 1, '\n');
  }
  lv_label_set_text(alert_type, title);
  lv_label_set_long_mode(alert_type, LV_LABEL_LONG_SROLL_CIRC);
  lv_obj_set_width(alert_type, 180);
  lv_obj_align(alert_type, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 16);

  /////////
  switch (category) {
    default: {
      lv_obj_t* alert_subject = lv_label_create(container1, nullptr);
      lv_obj_set_style_local_text_color(alert_subject, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
      lv_label_set_long_mode(alert_subject, LV_LABEL_LONG_BREAK);
      lv_obj_set_width(alert_subject, LV_HOR_RES - 20);
      lv_label_set_text(alert_subject, msg);
    } break;
    case Controllers::NotificationManager::Categories::IncomingCall: {
      lv_obj_set_height(container1, 108);
      lv_obj_t* alert_subject = lv_label_create(container1, nullptr);
      lv_obj_set_style_local_text_color(alert_subject, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
      lv_label_set_long_mode(alert_subject, LV_LABEL_LONG_BREAK);
      lv_obj_set_width(alert_subject, LV_HOR_RES - 20);
      lv_label_set_text(alert_subject, "Incoming call from");

      lv_obj_t* alert_caller = lv_label_create(container1, nullptr);
      lv_obj_align(alert_caller, alert_subject, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
      lv_label_set_long_mode(alert_caller, LV_LABEL_LONG_BREAK);
      lv_obj_set_width(alert_caller, LV_HOR_RES - 20);
      lv_label_set_text(alert_caller, msg);

      bt_accept = lv_btn_create(lv_scr_act(), nullptr);
      bt_accept->user_data = this;
      lv_obj_set_event_cb(bt_accept, CallEventHandler);
      lv_obj_set_size(bt_accept, 76, 76);
      lv_obj_align(bt_accept, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
      label_accept = lv_label_create(bt_accept, nullptr);
      lv_label_set_text(label_accept, Symbols::phone);
      lv_obj_set_style_local_bg_color(bt_accept, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);

      bt_reject = lv_btn_create(lv_scr_act(), nullptr);
      bt_reject->user_data = this;
      lv_obj_set_event_cb(bt_reject, CallEventHandler);
      lv_obj_set_size(bt_reject, 76, 76);
      lv_obj_align(bt_reject, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
      label_reject = lv_label_create(bt_reject, nullptr);
      lv_label_set_text(label_reject, Symbols::phoneSlash);
      lv_obj_set_style_local_bg_color(bt_reject, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

      bt_mute = lv_btn_create(lv_scr_act(), nullptr);
      bt_mute->user_data = this;
      lv_obj_set_event_cb(bt_mute, CallEventHandler);
      lv_obj_set_size(bt_mute, 76, 76);
      lv_obj_align(bt_mute, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
      label_mute = lv_label_create(bt_mute, nullptr);
      lv_label_set_text(label_mute, Symbols::volumMute);
      lv_obj_set_style_local_bg_color(bt_mute, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    } break;
  }

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");
}

void Notifications::NotificationItem::OnCallButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
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
