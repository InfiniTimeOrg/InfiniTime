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

int OnControlPointWriteCallback(uint16_t conn_handle, const struct ble_gatt_error* error, struct ble_gatt_attr* attr, void* arg) {
  auto client = static_cast<AppleNotificationCenterClient*>(arg);
  return client->OnControlPointWrite(conn_handle, error, attr);
}

AppleNotificationCenterClient::AppleNotificationCenterClient(Pinetime::System::SystemTask& systemTask,
                                                             Pinetime::Controllers::NotificationManager& notificationManager)
  : systemTask {systemTask}, notificationManager {notificationManager} {
}

bool AppleNotificationCenterClient::OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error* error, const ble_gatt_svc* service) {
  if (service == nullptr && error->status == BLE_HS_EDONE) {
    if (isDiscovered) {
      NRF_LOG_INFO("ANCS Discovery found, starting characteristics discovery");
      // DebugNotification("ANCS Discovery found, starting characteristics discovery");

      ble_gattc_disc_all_chrs(connectionHandle, ancsStartHandle, ancsEndHandle, OnANCSCharacteristicDiscoveredCallback, this);
    } else {
      NRF_LOG_INFO("ANCS not found");
      // DebugNotification("ANCS not found");
      onServiceDiscovered(connectionHandle);
    }
    return true;
  }

  if (service != nullptr && ble_uuid_cmp(&ancsUuid.u, &service->uuid.u) == 0) {
    NRF_LOG_INFO("ANCS discovered : 0x%x - 0x%x", service->start_handle, service->end_handle);
    // DebugNotification("ANCS discovered");
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
    // DebugNotification("ANCS Characteristic discovery ERROR");
    onServiceDiscovered(connectionHandle);
    return 0;
  }

  if (characteristic == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("ANCS Characteristic discovery complete");
    DebugNotification("ANCS Characteristic discovery complete");
    if (isCharacteristicDiscovered) {
      ble_gattc_disc_all_dscs(connectionHandle, notificationSourceHandle, ancsEndHandle, OnANCSDescriptorDiscoveryEventCallback, this);
    } if (isDataCharacteristicDiscovered) {
      // DebugNotification("ANCS Characteristic discovery complete: Data Source");
      ble_gattc_disc_all_dscs(connectionHandle, dataSourceHandle, 0xFFFF, OnANCSDescriptorDiscoveryEventCallback, this);
    }
    if (isCharacteristicDiscovered == isControlCharacteristicDiscovered && isCharacteristicDiscovered == isDataCharacteristicDiscovered) {
      onServiceDiscovered(connectionHandle);
    }
  } else {
    if (characteristic != nullptr) {
      if (ble_uuid_cmp(&notificationSourceChar.u, &characteristic->uuid.u) == 0) {
        NRF_LOG_INFO("ANCS Characteristic discovered: Notification Source");
        // DebugNotification("ANCS Characteristic discovered: Notification Source");
        notificationSourceHandle = characteristic->val_handle;
        isCharacteristicDiscovered = true;
      } else if (ble_uuid_cmp(&controlPointChar.u, &characteristic->uuid.u) == 0) {
        NRF_LOG_INFO("ANCS Characteristic discovered: Control Point");
        // DebugNotification("ANCS Characteristic discovered: Control Point");
        controlPointHandle = characteristic->val_handle;
        isControlCharacteristicDiscovered = true;
      } else if (ble_uuid_cmp(&dataSourceChar.u, &characteristic->uuid.u) == 0) {
        NRF_LOG_INFO("ANCS Characteristic discovered: Data Source");
        DebugNotification("ANCS Characteristic discovered: Data Source");
        dataSourceHandle = characteristic->val_handle;
        isDataCharacteristicDiscovered = true;
      }
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
        // DebugNotification("ANCS Descriptor discovered");
        notificationSourceDescriptorHandle = descriptor->handle;
        isDescriptorFound = true;
        uint8_t value[2] {1, 0};
        ble_gattc_write_flat(connectionHandle, notificationSourceDescriptorHandle, value, sizeof(value), NewAlertSubcribeCallback, this);
      }
    } else if (characteristicValueHandle == controlPointHandle && ble_uuid_cmp(&controlPointChar.u, &descriptor->uuid.u)) {
      if (controlPointDescriptorHandle == 0) {
        NRF_LOG_INFO("ANCS Descriptor discovered : %d", descriptor->handle);
        // DebugNotification("ANCS Descriptor discovered");
        controlPointDescriptorHandle = descriptor->handle;
        isControlDescriptorFound = true;      
      }
    } else if (characteristicValueHandle == dataSourceHandle && ble_uuid_cmp(&dataSourceChar.u, &descriptor->uuid.u)) {
      if (dataSourceDescriptorHandle == 0) {
        NRF_LOG_INFO("ANCS Descriptor discovered : %d", descriptor->handle);
        DebugNotification("ANCS Descriptor discovered: Data Source");
        dataSourceDescriptorHandle = descriptor->handle;
        isDataDescriptorFound = true;
        uint8_t value[2] {1, 0};
        ble_gattc_write_flat(connectionHandle, dataSourceDescriptorHandle, value, sizeof(value), NewAlertSubcribeCallback, this);
      }
    }
  } else {
    char errorStr[55];
    snprintf(errorStr, sizeof(errorStr), "ANCS Descriptor discovery ERROR: %d", error->status);
    NRF_LOG_INFO(errorStr);
    DebugNotification(errorStr);
    if (isDescriptorFound == isDataDescriptorFound)
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
  if (isDescriptorFound == isControlDescriptorFound && isDescriptorFound == isDataDescriptorFound)
    onServiceDiscovered(connectionHandle);

  return 0;
}

int AppleNotificationCenterClient::OnControlPointWrite(uint16_t /*connectionHandle*/, const ble_gatt_error* error, ble_gatt_attr* /*attribute*/) {
  if (error->status == 0) {
    NRF_LOG_INFO("ANCS Control Point write OK");
    // DebugNotification("ANCS Control Point write OK");
  } else {
    char errorStr[55];
    snprintf(errorStr, sizeof(errorStr), "ANCS Control Point ERROR: %d", error->status);
    NRF_LOG_INFO(errorStr);
    DebugNotification(errorStr);
  }
  return 0;
}

void AppleNotificationCenterClient::OnNotification(ble_gap_event* event) {
  if (event->notify_rx.attr_handle == notificationSourceHandle || event->notify_rx.attr_handle == notificationSourceDescriptorHandle) {
    NRF_LOG_INFO("ANCS Notification received");
    uint8_t eventId;
    uint8_t eventFlags;
    uint8_t category;
    uint8_t categoryCount;
    uint32_t notificationUuid;

    os_mbuf_copydata(event->notify_rx.om, 0, 1, &eventId);
    os_mbuf_copydata(event->notify_rx.om, 1, 1, &eventFlags);
    os_mbuf_copydata(event->notify_rx.om, 2, 1, &category);
    os_mbuf_copydata(event->notify_rx.om, 3, 1, &categoryCount);
    os_mbuf_copydata(event->notify_rx.om, 4, 4, &notificationUuid);

    // bool silent = eventFlags & static_cast<uint8_t>(EventFlags::Silent);
    // bool important = eventFlags & static_cast<uint8_t>(EventFlags::Important);
    bool preExisting = eventFlags & static_cast<uint8_t>(EventFlags::PreExisting);
    // bool positiveAction = eventFlags & static_cast<uint8_t>(EventFlags::PositiveAction);
    // bool negativeAction = eventFlags & static_cast<uint8_t>(EventFlags::NegativeAction);

    if (eventId != static_cast<uint8_t>(EventIds::Added) && preExisting != true) {
      return;
    }

    // Request ANCS more info
    uint8_t titleSize = 30;
    BYTE request[8];
    request[0] = 0x00; // Command ID: Get Notification Attributes
    request[1] = (uint8_t) (notificationUuid & 0xFF);
    request[2] = (uint8_t) ((notificationUuid >> 8) & 0xFF);
    request[3] = (uint8_t) ((notificationUuid >> 16) & 0xFF);
    request[4] = (uint8_t) ((notificationUuid >> 24) & 0xFF);
    request[5] = 0x01;
    // request[6] = 0x00;
    request[6] = (titleSize & 0xFF);
    request[7] = ((titleSize >> 8) & 0xFF);

    ble_gattc_write_flat(event->notify_rx.conn_handle, controlPointHandle, request, sizeof(request), OnControlPointWriteCallback, this);

    // NotificationManager::Notification notif;
    // char uuidStr[55];
    // snprintf(uuidStr, sizeof(uuidStr), "iOS Notif.:%08lx\nEvID: %d\nCat: %d", notificationUuid, eventId, category);
    // notif.message = std::array<char, 101> {};
    // std::strncpy(notif.message.data(), uuidStr, notif.message.size() - 1);
    // notif.message[10] = '\0'; // Seperate Title and Message
    // notif.message[notif.message.size() - 1] = '\0'; // Ensure null-termination
    // notif.size = std::min(std::strlen(uuidStr), notif.message.size());
    // notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
    // notificationManager.Push(std::move(notif));

    systemTask.PushMessage(Pinetime::System::Messages::OnNewNotification);
  } else if (event->notify_rx.attr_handle == dataSourceHandle || event->notify_rx.attr_handle == dataSourceDescriptorHandle) {
    uint16_t titleSize;
    uint16_t title;

    os_mbuf_copydata(event->notify_rx.om, 6, 2, &titleSize);
    os_mbuf_copydata(event->notify_rx.om, 8, titleSize, &title);

    // char mesgStr[90];
    // snprintf(mesgStr, sizeof(mesgStr), "ANCS Data Source received\nTitle: %d", title);

    // NRF_LOG_INFO(mesgStr);
    // DebugNotification(mesgStr);
    DebugNotification("ANCS Data Source received");
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
  // DebugNotification("[ANCS] Starting discovery");
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