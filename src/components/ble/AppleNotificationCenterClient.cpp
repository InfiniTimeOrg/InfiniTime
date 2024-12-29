#include "components/ble/AppleNotificationCenterClient.h"
#include <algorithm>
#include "components/ble/NotificationManager.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

int OnDiscoveryEventCallback(uint16_t conn_handle, const struct ble_gatt_error* error, const struct ble_gatt_svc* service, void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnDiscoveryEvent(conn_handle, error, service);
}

int OnANCSCharacteristicDiscoveredCallback(uint16_t conn_handle,
                                           const struct ble_gatt_error* error,
                                           const struct ble_gatt_chr* chr,
                                           void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnCharacteristicsDiscoveryEvent(conn_handle, error, chr);
}

int OnANCSDescriptorDiscoveryEventCallback(uint16_t conn_handle,
                                           const struct ble_gatt_error* error,
                                           uint16_t chr_val_handle,
                                           const struct ble_gatt_dsc* dsc,
                                           void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnDescriptorDiscoveryEventCallback(conn_handle, error, chr_val_handle, dsc);
}

int NewAlertSubcribeCallback(uint16_t conn_handle, const struct ble_gatt_error* error, struct ble_gatt_attr* attr, void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnNewAlertSubcribe(conn_handle, error, attr);
}

AppleNotificationCenterClient::AppleNotificationCenterClient(Pinetime::System::SystemTask& systemTask,
                                                             Pinetime::Controllers::NotificationManager& notificationManager)
  : systemTask {systemTask}, notificationManager {notificationManager} {
}

bool AppleNotificationCenterClient::OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error* error, const ble_gatt_svc* service) {
  if (service == nullptr && error->status == BLE_HS_EDONE) {
    if (isDiscovered) {
      NRF_LOG_INFO("ANCS Discovery found, starting characteristics discovery");
      DebugNotification("ANCS Discovery found, starting characteristics discovery");

      ble_gattc_disc_all_chrs(connectionHandle, ancsStartHandle, ancsEndHandle, OnANCSCharacteristicDiscoveredCallback, this);
    } else {
      NRF_LOG_INFO("ANCS not found");
      DebugNotification("ANCS not found");
      onServiceDiscovered(connectionHandle);
    }
    return true;
  }

  if (service != nullptr && ble_uuid_cmp(&ancsUuid.u, &service->uuid.u) == 0) {
    NRF_LOG_INFO("ANCS discovered : 0x%x - 0x%x", service->start_handle, service->end_handle);
    DebugNotification("ANCS discovered");
    ancsStartHandle = service->start_handle;
    ancsEndHandle = service->end_handle;
    isDiscovered = true;
  }
  return false;
}

int AppleNotificationCenterClient::OnCharacteristicsDiscoveryEvent(uint16_t connectionHandle,
                                                                   const ble_gatt_error* error,
                                                                   const ble_gatt_chr* characteristic) {
  if (error->status != 0 && error->status != BLE_HS_EDONE) {
    NRF_LOG_INFO("ANCS Characteristic discovery ERROR");
    DebugNotification("ANCS Characteristic discovery ERROR");
    onServiceDiscovered(connectionHandle);
    return 0;
  }

  if (characteristic == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("ANCS Characteristic discovery complete");
    DebugNotification("ANCS Characteristic discovery complete");
    if (isCharacteristicDiscovered) {
      ble_gattc_disc_all_dscs(connectionHandle, notificationSourceHandle, ancsEndHandle, OnANCSDescriptorDiscoveryEventCallback, this);
    } else {
      onServiceDiscovered(connectionHandle);
    }
  } else {
    if (characteristic != nullptr && ble_uuid_cmp(&notificationSourceChar.u, &characteristic->uuid.u) == 0) {
      NRF_LOG_INFO("ANCS Characteristic discovered: Notification Source");
      DebugNotification("ANCS Characteristic discovered: Notification Source");
      notificationSourceHandle = characteristic->val_handle;
      // notificationSourceDefHandle = characteristic->def_handle;
      isCharacteristicDiscovered = true;
    }
  }
  return 0;
}

int AppleNotificationCenterClient::OnDescriptorDiscoveryEventCallback(uint16_t connectionHandle,
                                                                      const ble_gatt_error* error,
                                                                      uint16_t characteristicValueHandle,
                                                                      const ble_gatt_dsc* descriptor) {
  if (error->status == 0) {
    if (characteristicValueHandle == notificationSourceHandle && ble_uuid_cmp(&notificationSourceChar.u, &descriptor->uuid.u)) {
      if (notificationSourceDescriptorHandle == 0) {
        NRF_LOG_INFO("ANCS Descriptor discovered : %d", descriptor->handle);
        DebugNotification("ANCS Descriptor discovered");
        notificationSourceDescriptorHandle = descriptor->handle;
        isDescriptorFound = true;
        uint8_t value[2] {1, 0};
        ble_gattc_write_flat(connectionHandle, notificationSourceDescriptorHandle, value, sizeof(value), NewAlertSubcribeCallback, this);
      }
    }
  } else {
    if (!isDescriptorFound)
      onServiceDiscovered(connectionHandle);
  }
  return 0;
}

int AppleNotificationCenterClient::OnNewAlertSubcribe(uint16_t connectionHandle, const ble_gatt_error* error, ble_gatt_attr* /*attribute*/) {
  if (error->status == 0) {
    NRF_LOG_INFO("ANCS New alert subscribe OK");
    DebugNotification("ANCS New alert subscribe OK");
  } else {
    NRF_LOG_INFO("ANCS New alert subscribe ERROR");
    DebugNotification("ANCS New alert subscribe ERROR");
  }
  onServiceDiscovered(connectionHandle);

  return 0;
}

void AppleNotificationCenterClient::OnNotification(ble_gap_event* event) {
  if (event->notify_rx.attr_handle == notificationSourceHandle) {
    NRF_LOG_INFO("ANCS Notification received");
    uint8_t eventId;
    uint8_t eventFlag;
    uint8_t category;
    uint8_t categoryCount;
    uint32_t notificationUuid;

    os_mbuf_copydata(event->notify_rx.om, 0, 1, &eventId);
    os_mbuf_copydata(event->notify_rx.om, 1, 1, &eventFlag);
    os_mbuf_copydata(event->notify_rx.om, 2, 1, &category);
    os_mbuf_copydata(event->notify_rx.om, 3, 1, &categoryCount);
    os_mbuf_copydata(event->notify_rx.om, 4, 4, &notificationUuid);

    if (eventId != static_cast<uint8_t>(EventIds::Added)) {
      return;
    }

    // Request ANCS more info
    // uint8_t request[] = {
    //   0x00, // Command ID: Get Notification Attributes
    //   (uint8_t) (notificationUuid & 0xFF),
    //   (uint8_t) ((notificationUuid >> 8) & 0xFF),
    //   (uint8_t) ((notificationUuid >> 16) & 0xFF),
    //   (uint8_t) ((notificationUuid >> 24) & 0xFF),
    //   0x01,
    //   0x00,
    //   0xFF,
    //   0xFF, // Title (max length 65535)
    //   0x03,
    //   0x00,
    //   0xFF,
    //   0xFF // Message (max length 65535)
    // };
    // ble_gattc_write_flat(event->notify_rx.conn_handle, controlPointHandle, request, sizeof(request), nullptr, nullptr);

    NotificationManager::Notification notif;
    char uuidStr[55];
    snprintf(uuidStr, sizeof(uuidStr), "iOS Notif.:%08lx\nEvID: %d\nCat: %d", notificationUuid, eventId, category);
    notif.message = std::array<char, 101> {};
    std::strncpy(notif.message.data(), uuidStr, notif.message.size() - 1);
    notif.message[10] = '\0'; // Seperate Title and Message
    notif.message[notif.message.size() - 1] = '\0'; // Ensure null-termination
    notif.size = std::min(std::strlen(uuidStr), notif.message.size());
    notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
    notificationManager.Push(std::move(notif));

    systemTask.PushMessage(Pinetime::System::Messages::OnNewNotification);
  }
}

void AppleNotificationCenterClient::Reset() {
  ancsStartHandle = 0;
  ancsEndHandle = 0;
  gattStartHandle = 0;
  gattEndHandle = 0;
  serviceChangedHandle = 0;
  serviceChangedDescriptorHandle = 0;
  notificationSourceHandle = 0;
  notificationSourceDescriptorHandle = 0;
  controlPointHandle = 0;
  controlPointDescriptorHandle = 0;
  dataSourceHandle = 0;
  dataSourceDescriptorHandle = 0;
  isGattDiscovered = false;
  isGattCharacteristicDiscovered = false;
  isGattDescriptorFound = false;
  isDiscovered = false;
  isCharacteristicDiscovered = false;
  isDescriptorFound = false;
  isControlCharacteristicDiscovered = false;
  isControlDescriptorFound = false;
  isDataCharacteristicDiscovered = false;
  isDataDescriptorFound = false;
}

void AppleNotificationCenterClient::Discover(uint16_t connectionHandle, std::function<void(uint16_t)> onServiceDiscovered) {
  NRF_LOG_INFO("[ANCS] Starting discovery");
  DebugNotification("[ANCS] Starting discovery");
  this->onServiceDiscovered = onServiceDiscovered;
  ble_gattc_disc_svc_by_uuid(connectionHandle, &ancsUuid.u, OnDiscoveryEventCallback, this);
}

void AppleNotificationCenterClient::DebugNotification(const char* msg) const {
  NRF_LOG_INFO("[ANCS DEBUG] %s", msg);

  NotificationManager::Notification notif;
  std::strncpy(notif.message.data(), msg, notif.message.size() - 1);
  notif.message[notif.message.size() - 1] = '\0'; // Ensure null-termination
  notif.size = std::min(std::strlen(msg), notif.message.size());
  notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
  notificationManager.Push(std::move(notif));

  systemTask.PushMessage(Pinetime::System::Messages::OnNewNotification);
}