
#include <hal/nrf_rtc.h>
#include "NotificationManager.h"
#include <SystemTask/SystemTask.h>

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
    static const size_t maxBufferSize{21};
    static const size_t maxMessageSize{18};
    size_t bufferSize = min(OS_MBUF_PKTLEN(ctxt->om), maxBufferSize);

    uint8_t data[bufferSize];
    os_mbuf_copydata(ctxt->om, 0, bufferSize, data);

    char *s = (char *) &data[3];
    auto messageSize = min(maxMessageSize, (bufferSize-3));

    for (uint i = 0; i < messageSize-1; i++) {
      if (s[i] == 0x00) {
        s[i] = 0x0A;
      }
    }
    s[messageSize-1] = '\0';

    m_notificationManager.Push(Pinetime::Controllers::NotificationManager::Categories::SimpleAlert, s, messageSize);
    m_systemTask.PushMessage(Pinetime::System::SystemTask::Messages::OnNewNotification);
  }
  return 0;
}
