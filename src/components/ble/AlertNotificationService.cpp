
#include <hal/nrf_rtc.h>
#include "NotificationManager.h"
#include <systemtask/SystemTask.h>

#include "AlertNotificationService.h"
#include <cstring>

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t AlertNotificationService::ansUuid;
constexpr ble_uuid16_t AlertNotificationService::ansCharUuid;


int AlertNotificationCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
  auto anService = static_cast<AlertNotificationService*>(arg);
  return anService->OnAlert(conn_handle, attr_handle, ctxt);
}

void AlertNotificationService::Init() {
  int res;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

AlertNotificationService::AlertNotificationService ( System::SystemTask& systemTask, NotificationManager& notificationManager )
  : characteristicDefinition{
                {
                        .uuid = (ble_uuid_t *) &ansCharUuid,
                        .access_cb = AlertNotificationCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_WRITE
                },
                {
                  0
                }
        },
        serviceDefinition{
                {
                        /* Device Information Service */
                        .type = BLE_GATT_SVC_TYPE_PRIMARY,
                        .uuid = (ble_uuid_t *) &ansUuid,
                        .characteristics = characteristicDefinition
                },
                {
                        0
                },
        }, m_systemTask{systemTask}, m_notificationManager{notificationManager} {
}

int AlertNotificationService::OnAlert(uint16_t conn_handle, uint16_t attr_handle,
                                                    struct ble_gatt_access_ctxt *ctxt) {

  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    // TODO implement this with more memory safety (and constexpr)
    size_t notifSize = OS_MBUF_PKTLEN(ctxt->om);
    uint8_t *data = (uint8_t*) malloc(notifSize + 1);
    data[notifSize] = '\0';
    os_mbuf_copydata(ctxt->om, 0, notifSize, data);
    char alertCategory = (char) data[0];
    char *s = (char *) &data[1];
    NRF_LOG_INFO("DATA : %s", s);

    Pinetime::Controllers::NotificationManager::Categories messageCategory;
    switch(alertCategory) {
      case ALERT_UNKNOWN:
        messageCategory = Pinetime::Controllers::NotificationManager::Categories::Unknown;
        break;
      case ALERT_SIMPLE_ALERT:
        messageCategory = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
        break;
      case ALERT_EMAIL:
        messageCategory = Pinetime::Controllers::NotificationManager::Categories::Email;
        break;
      case ALERT_NEWS:
        messageCategory = Pinetime::Controllers::NotificationManager::Categories::News;
        break;
      case ALERT_INCOMING_CALL:
        messageCategory = Pinetime::Controllers::NotificationManager::Categories::IncomingCall;
        break;
      case ALERT_MISSED_CALL:
        messageCategory = Pinetime::Controllers::NotificationManager::Categories::MissedCall;
        break;
      case ALERT_SMS:
        messageCategory = Pinetime::Controllers::NotificationManager::Categories::Sms;
        break;
      case ALERT_VOICE_MAIL:
        messageCategory = Pinetime::Controllers::NotificationManager::Categories::VoiceMail;
        break;
      case ALERT_SCHEDULE:
        messageCategory = Pinetime::Controllers::NotificationManager::Categories::Schedule;
        break;
      case ALERT_HIGH_PRIORITY_ALERT:
        messageCategory = Pinetime::Controllers::NotificationManager::Categories::HighProriotyAlert;
        break;
      case ALERT_INSTANT_MESSAGE:
        messageCategory = Pinetime::Controllers::NotificationManager::Categories::InstantMessage;
        break;
      default:
        messageCategory = Pinetime::Controllers::NotificationManager::Categories::Unknown;
    }
    m_notificationManager.Push(messageCategory, s, notifSize);
    m_systemTask.PushMessage(Pinetime::System::SystemTask::Messages::OnNewNotification);
  }
  return 0;
}
