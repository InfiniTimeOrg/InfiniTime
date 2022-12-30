#include "components/ble/AlertNotificationService.h"
#include <hal/nrf_rtc.h>
#include <cstring>
#include <algorithm>
#include "components/ble/NotificationManager.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t AlertNotificationService::ansUuid;
constexpr ble_uuid16_t AlertNotificationService::ansCharUuid;
constexpr ble_uuid128_t AlertNotificationService::notificationEventUuid;

int AlertNotificationCallback(uint16_t /*conn_handle*/, uint16_t /*attr_handle*/, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto anService = static_cast<AlertNotificationService*>(arg);
  return anService->OnAlert(ctxt);
}

void AlertNotificationService::Init() {
  int res;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

AlertNotificationService::AlertNotificationService(System::SystemTask& systemTask, NotificationManager& notificationManager)
  : characteristicDefinition {{.uuid = &ansCharUuid.u, .access_cb = AlertNotificationCallback, .arg = this, .flags = BLE_GATT_CHR_F_WRITE},
                              {.uuid = &notificationEventUuid.u,
                               .access_cb = AlertNotificationCallback,
                               .arg = this,
                               .flags = BLE_GATT_CHR_F_NOTIFY,
                               .val_handle = &eventHandle},
                              {0}},
    serviceDefinition {
      {/* Device Information Service */
       .type = BLE_GATT_SVC_TYPE_PRIMARY,
       .uuid = &ansUuid.u,
       .characteristics = characteristicDefinition},
      {0},
    },
    systemTask {systemTask},
    notificationManager {notificationManager} {
}

int AlertNotificationService::OnAlert(struct ble_gatt_access_ctxt* ctxt) {
  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    constexpr size_t stringTerminatorSize = 1; // end of string '\0'
    constexpr size_t headerSize = 3;
    const auto maxMessageSize {NotificationManager::MaximumMessageSize()};
    const auto maxBufferSize {maxMessageSize + headerSize};

    // Ignore notifications with empty message
    const auto packetLen = OS_MBUF_PKTLEN(ctxt->om);
    if (packetLen <= headerSize) {
      return 0;
    }

    size_t bufferSize = std::min(packetLen + stringTerminatorSize, maxBufferSize);
    auto messageSize = std::min(maxMessageSize, (bufferSize - headerSize));
    Categories category;

    NotificationManager::Notification notif;
    os_mbuf_copydata(ctxt->om, headerSize, messageSize - 1, notif.message.data());
    os_mbuf_copydata(ctxt->om, 0, 1, &category);
    notif.message[messageSize - 1] = '\0';
    notif.size = messageSize;

    // TODO convert all ANS categories to NotificationController categories
    switch (category) {
      case Categories::Call:
        notif.category = Pinetime::Controllers::NotificationManager::Categories::IncomingCall;
        break;
      default:
        notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
        break;
    }

    auto event = Pinetime::System::Messages::OnNewNotification;
    notificationManager.Push(std::move(notif));
    systemTask.PushMessage(event);
  }
  return 0;
}

void AlertNotificationService::AcceptIncomingCall() {
  auto response = IncomingCallResponses::Answer;
  auto* om = ble_hs_mbuf_from_flat(&response, 1);

  uint16_t connectionHandle = systemTask.nimble().connHandle();

  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }

  ble_gattc_notify_custom(connectionHandle, eventHandle, om);
}

void AlertNotificationService::RejectIncomingCall() {
  auto response = IncomingCallResponses::Reject;
  auto* om = ble_hs_mbuf_from_flat(&response, 1);

  uint16_t connectionHandle = systemTask.nimble().connHandle();

  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }

  ble_gattc_notify_custom(connectionHandle, eventHandle, om);
}

void AlertNotificationService::MuteIncomingCall() {
  auto response = IncomingCallResponses::Mute;
  auto* om = ble_hs_mbuf_from_flat(&response, 1);

  uint16_t connectionHandle = systemTask.nimble().connHandle();

  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }

  ble_gattc_notify_custom(connectionHandle, eventHandle, om);
}
