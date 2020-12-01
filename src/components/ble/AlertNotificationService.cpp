#include "AlertNotificationService.h"
#include <hal/nrf_rtc.h>
#include <cstring>
#include <algorithm>
#include "NotificationManager.h"
#include "systemtask/SystemTask.h"

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
        }, systemTask{systemTask}, notificationManager{notificationManager} {
}

int AlertNotificationService::OnAlert(uint16_t conn_handle, uint16_t attr_handle,
                                                    struct ble_gatt_access_ctxt *ctxt) {
  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    constexpr size_t stringTerminatorSize = 1; // end of string '\0'
    constexpr size_t headerSize = 3;
    const auto maxMessageSize {NotificationManager::MaximumMessageSize()};
    const auto maxBufferSize{maxMessageSize + headerSize};

    const auto dbgPacketLen = OS_MBUF_PKTLEN(ctxt->om);
    size_t bufferSize = std::min(dbgPacketLen + stringTerminatorSize, maxBufferSize);
    auto messageSize = std::min(maxMessageSize, (bufferSize-headerSize));

    NotificationManager::Notification notif;
    os_mbuf_copydata(ctxt->om, headerSize, messageSize-1, notif.message.data());
    notif.message[messageSize-1] = '\0';
    notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
    notificationManager.Push(std::move(notif));

    systemTask.PushMessage(Pinetime::System::SystemTask::Messages::OnNewNotification);
  }
  return 0;
}
